#include "RenderingManager.h"
#include "ConstantBufferTypes.h"
#include "Scene.h"
#include "ErrorLogger.h"
#include "GameWindow.h"
#include <cassert>
#include "BLASBuilder.h"
#include "AssetManager.h"
#include "LightManager.h"
#include "PhysicsManager.h"
#include "DX12_Data.h"

namespace ECS
{
	RenderingManager::RenderingManager()
	{
		m_ambientColor = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
		m_exposure = 1.0f;
		m_gamma = 2.2f;
	}

	RenderingManager::~RenderingManager()
	{
		GetDX12().WaitForGPU(GetDX12().GetCommandQueue(), GetDX12().fence.Get(), GetDX12().fenceEvent, GetDX12().fenceValue);
		m_reflectionsTexture.reset();
		m_AOTexture.reset();
		m_bindlessAlbedoTextures.reset();
		m_rtEntityHandle.reset();
	}

	bool RenderingManager::Initialize(GameWindow& game_window, int width, int height)
	{
		m_screenWidth = game_window.GetScreenWidth();
		m_screenHeight = game_window.GetScreenHeight();

		GetDX12().Initialize(game_window.GetWindow(), game_window.GetScreenWidth(), game_window.GetScreenHeight());
		if (!m_gui.Initialize(game_window.GetSDLWindow(), GetDX12().GetDevice(), GetDX12().GetCommandQueue(), GetDX12().GetRtvHeap(), GetDX12().GetDescriptorAllocator()))
		{
			ErrorLogger::Log("Failed to initialize ImGui!");
			return false;
		}

		m_computeSkinning.Initialize();
		return true;
	}

	void RenderingManager::InitializeRenderTargets(Scene* scene)
	{
		hdr_map1.Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetDescriptorAllocator(), "Data/HDR/qwantani_dusk_2_puresky_2k.hdr");

		m_gBuffer.Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), m_screenWidth, m_screenHeight);
		m_cubeMap1.Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), 512, 512);
		m_irradianceMap.Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), 64, 64);
		m_prefilterMap.Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), 512, 512, 5);

		std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
		m_brdfMap = std::make_unique<RenderTargetTexture>(1);
		m_brdfMap->Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), 512, 512, formats, 1);

		std::vector<DXGI_FORMAT> LightPassformats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
		m_lightPassRenderTarget = std::make_unique<RenderTargetTexture>(1);
		m_lightPassRenderTarget->Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), 
			GetDX12().GetScreenWidth(), GetDX12().GetScreenHeight(), LightPassformats, 1);

		// Ray traced reflections UAV texture initialization
		m_reflectionsTexture = std::make_unique<Texture12>();
		TextureDesc textDesc;
		textDesc.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textDesc.width = m_screenWidth / 2;
		textDesc.height = m_screenHeight / 2;
		textDesc.slices = 1;
		textDesc.viewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		m_reflectionsTexture->CreateTextureUAV(GetDX12().GetDevice(), GetDX12().GetDescriptorAllocator(), textDesc);


		// Ray traced ambient occlusion UAV texture initialization
		m_AOTexture = std::make_unique<Texture12>();
		textDesc.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textDesc.width = m_screenWidth / 2;
		textDesc.height = m_screenHeight / 2;
		textDesc.slices = 1;
		textDesc.viewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		m_AOTexture->CreateTextureUAV(GetDX12().GetDevice(), GetDX12().GetDescriptorAllocator(), textDesc);
	}

	void RenderingManager::InitializeShadowTextures(Scene* scene)
	{
		// Get all the light components in the scene
		auto lightsView = scene->GetRegistry().view<LightComponent>();
		std::size_t totalLights = lightsView.size();

		m_shadowsTexture = scene->GetLightManager()->GetShadowsTexturePtr();
		TextureDesc textDesc;
		textDesc.format = DXGI_FORMAT_R16_FLOAT;
		textDesc.width = GetDX12().GetScreenWidth();
		textDesc.height = GetDX12().GetScreenHeight();
		textDesc.slices = (UINT16)totalLights;
		textDesc.viewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		m_shadowsTexture->CreateTextureUAV(GetDX12().GetDevice(), GetDX12().GetDescriptorAllocator(), textDesc);
	}

	// Populate ray tracing data such as vertex/index buffers, mesh data offsets and bindless textures
	void RenderingManager::PopulateRayTracingData(Scene* scene)
	{
		m_bindlessAlbedoTextures = std::make_unique<Texture12>();
		m_rtEntityHandle = std::make_unique <RTEntityHandle>();
		m_textureMapping = std::make_unique<std::map<uint32_t, std::shared_ptr<Texture12>>>();

		UINT id = 0;
		uint32_t vertexSize = 0;
		uint32_t indicesSize = 0;

		auto group = scene->GetRegistry().group<>(entt::get<ECS::TransformComponent, ECS::RenderComponent>);
		for (auto [entity, transformComponent, renderComponent] : group.each())
		{
			if (renderComponent.meshType != ECS::STATIC_MESH && renderComponent.meshType != ECS::MESH_TYPE::SKELETAL_MESH)
				continue;

			auto albedoTexture = renderComponent.material->albedoTexture;
			m_textureMapping->emplace(id, albedoTexture); // emplace the textures for each entity participating in ray tracing

			ECS::RTMeshDataOffsets dataOffets;
			dataOffets.vertexOffset = vertexSize;
			dataOffets.indexOffset = indicesSize;
			vertexSize += renderComponent.mesh->cpuMesh->vertices.size();
			indicesSize += renderComponent.mesh->cpuMesh->indices.size();
			dataOffets.padding = DirectX::XMFLOAT2(0.0f, 0.0f);
			rt_meshDataOffsests.push_back(dataOffets);

			for (size_t i = 0; i < renderComponent.mesh->cpuMesh->vertices.size(); ++i)
			{
				ECS::RTVertexData data;
				data.position = renderComponent.mesh->cpuMesh->vertices[i].pos;
				data.uv = renderComponent.mesh->cpuMesh->vertices[i].texCoord;
				data.padding1 = 1.0f;
				data.padding2 = DirectX::XMFLOAT2(1.0f, 1.0f);
				rt_vertexData.push_back(data);
			}
			for (size_t i = 0; i < renderComponent.mesh->cpuMesh->indices.size(); ++i)
			{
				ECS::RTIndexData data;
				data.indices = renderComponent.mesh->cpuMesh->indices[i];
				rt_indexData.push_back(data);
			}
			id++;
	
		}
		m_totalEntities = id;

		if (!m_textureMapping->empty())
		{
			m_bindlessAlbedoTextures->CreateBindlessTexture(GetDX12().GetDevice(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), m_textureMapping.get());

			m_rtEntityHandle->rtVertexGpuData.Initialize(GetDX12().GetDevice(), rt_vertexData.size());
			auto allocator = GetDX12().GetDescriptorAllocator()->Allocate();
			m_rtEntityHandle->cpuVertexHandle = allocator.cpuHandle;
			m_rtEntityHandle->gpuVertexHandle = allocator.gpuHandle;
			m_rtEntityHandle->rtVertexGpuData.CreateSRV(GetDX12().GetDevice(), m_rtEntityHandle->cpuVertexHandle);

			m_rtEntityHandle->rtIndexGpuData.Initialize(GetDX12().GetDevice(), rt_indexData.size());
			allocator = GetDX12().GetDescriptorAllocator()->Allocate();
			m_rtEntityHandle->cpuIndexHandle = allocator.cpuHandle;
			m_rtEntityHandle->gpuIndexHandle = allocator.gpuHandle;
			m_rtEntityHandle->rtIndexGpuData.CreateSRV(GetDX12().GetDevice(), m_rtEntityHandle->cpuIndexHandle);

			m_rtEntityHandle->rtMeshDataOffsets.Initialize(GetDX12().GetDevice(), m_totalEntities);
			allocator = GetDX12().GetDescriptorAllocator()->Allocate();
			m_rtEntityHandle->cpuOffsetsHandle = allocator.cpuHandle;
			m_rtEntityHandle->gpuOffsetsHandle = allocator.gpuHandle;
			m_rtEntityHandle->rtMeshDataOffsets.CreateSRV(GetDX12().GetDevice(), m_rtEntityHandle->cpuOffsetsHandle);

			m_rtEntityHandle->rtVertexGpuData.UploadData(GetDX12().GetCmdList(), rt_vertexData);
			m_rtEntityHandle->rtIndexGpuData.UploadData(GetDX12().GetCmdList(), rt_indexData);
			m_rtEntityHandle->rtMeshDataOffsets.UploadData(GetDX12().GetCmdList(), rt_meshDataOffsests);

			m_rtEntityHandle->rtVertexGpuData.GetResource()->TransitionState(GetDX12().GetCmdList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			m_rtEntityHandle->rtIndexGpuData.GetResource()->TransitionState(GetDX12().GetCmdList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			m_rtEntityHandle->rtMeshDataOffsets.GetResource()->TransitionState(GetDX12().GetCmdList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
	}

	void RenderingManager::CreateSBTs(Scene* scene)
	{
		GetDX12().CreateSBT(GetDX12().rtReflections_dispatchDesc, 1, GetDX12().GetRayTracedReflectionsResources(), GetDX12().GetScreenWidth() / 2, GetDX12().GetScreenHeight() / 2);
		GetDX12().CreateSBT(GetDX12().rtAO_dispatchDesc, 1, GetDX12().GetRayTracedAOResources(), GetDX12().GetScreenWidth() / 2, GetDX12().GetScreenHeight() / 2);
		GetDX12().CreateSBT(GetDX12().rtShadows_dispatchDesc, 1, GetDX12().GetRayTracedShadowsResources(), GetDX12().GetScreenWidth(), GetDX12().GetScreenHeight());
	}

	void RenderingManager::BuildTLAS(Scene* scene)
	{
		// Build TLAS for raytracing
		m_tlasBuilder.Build(scene);
	}

	void RenderingManager::RefitBLAS(Scene* scene)
	{
		BLASBuilder blas_builder;
		auto group = scene->GetRegistry().group<>(entt::get<RenderComponent, AnimatorComponent>);

		bool bAnyRefit = false;
		for (auto entity : group)
		{
			auto& renderComponent = group.get<RenderComponent>(entity);

			if (renderComponent.meshType == SKELETAL_MESH)
			{
				blas_builder.Refit(GetDX12().GetDevice(), GetDX12().GetCmdList(), renderComponent);
				bAnyRefit = true;
			}
		}
		if (bAnyRefit)
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
			GetDX12().GetCmdList()->ResourceBarrier(1, &barrier);
		}
	}

	DX12& RenderingManager::GetDX12()
	{
		return m_dx12;
	}

	GFXGui& RenderingManager::GetGFXGui()
	{
		return m_gui;
	}

	GBuffer& RenderingManager::GetGbuffer()
	{
		return m_gBuffer;
	}

	void RenderingManager::ResetRenderTargets()
	{
		m_gBuffer.ResetRenderTargets(GetDX12().GetCmdList());
		m_brdfMap->Reset(GetDX12().GetCmdList());
		m_lightPassRenderTarget->Reset(GetDX12().GetCmdList());
	}

	void RenderingManager::SetRenderTarget(RenderTargetTexture& renderTarget, float* clearColor)
	{
		renderTarget.SetRenderTarget(GetDX12().GetCmdList(), GetDX12().dsvHandle, clearColor);
	}

	void RenderingManager::RenderPbrMaps(Camera& camera)
	{
		if (bRenderPbrMaps)
		{
			m_cubeMap1.Render(GetDX12(), camera, GetDX12().pipelineState_Cubemap.Get(), 8, hdr_map1.GetHDRtexture()->GetGPUHandle());
			// Render the irradiance map
			m_irradianceMap.Render(GetDX12(), camera, GetDX12().pipelineState_IrradianceConv.Get(), 9, m_cubeMap1.GetCubeMapRenderTargetTexture()->GetSrvGpuHandle(0));
			// Render the prefilter map
			m_prefilterMap.RenderMips(GetDX12(), camera, GetDX12().pipelineState_Prefilter.Get(), 9, m_cubeMap1.GetCubeMapRenderTargetTexture()->GetSrvGpuHandle(0));
			// Render the brdf map
			RenderBRDF();
			bRenderPbrMaps = false;
		}
	}

	void RenderingManager::RenderGbuffer(Scene* scene, entt::entity& entity, TransformComponent& transformComponent, RenderComponent& renderComponent)
	{
		if (!scene)
			return;

		CB_VS_SimpleShader vsCB = {};
		CB_VS_Per_Object_Shader per_object_CB = {};
		CB_PS_Material psMaterialCB = {};
		CB_Shader_Camera psCameraCB = {};

		GetDX12().GetCmdList()->SetPipelineState(GetDX12().pipelineState_Gbuffer.Get());
		vsCB.projectionMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(scene->GetCamera().GetProjectionMatrix()));
		vsCB.viewMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(scene->GetCamera().GetViewMatrix()));
		vsCB.worldMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(transformComponent.worldMatrix));

		auto& cpuMesh = renderComponent.mesh->cpuMesh;
		std::size_t vertexCount = cpuMesh->vertices.size();
		per_object_CB.worldMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(transformComponent.worldMatrix));
		per_object_CB.vertexCount = vertexCount;
		per_object_CB.HasAnim = renderComponent.hasAnimation;


		if (renderComponent.meshType == ECS::MESH_TYPE::LIGHT)
		{
			if (scene->GetRegistry().all_of<ECS::LightComponent>(entity))
			{
				ECS::LightComponent& lightComponent = scene->GetRegistry().get<ECS::LightComponent>(entity);
				psMaterialCB.color = DirectX::XMFLOAT4(lightComponent.color.x, lightComponent.color.y, lightComponent.color.z, 1.0f);
			}
		}
		else
			psMaterialCB.color = renderComponent.material->baseColor;
		psMaterialCB.hasTextures = renderComponent.hasTextures;
		psMaterialCB.metalness = renderComponent.material->metalness;
		psMaterialCB.roughness = renderComponent.material->roughness;
		psMaterialCB.useAlbedo = renderComponent.material->useAlbedoMap;
		psMaterialCB.useNormals = renderComponent.material->useNormalMap;
		psMaterialCB.useRoughnessMetal = renderComponent.material->useMetalRoughnessMap;
		psMaterialCB.padding = 0.0f;

		psCameraCB.cameraPos = scene->GetCamera().pos;
		psCameraCB.padding1 = 0.0f;

		if (GetDX12().dynamicCB)
		{
			auto* cmdList = GetDX12().GetCmdList();
			auto* dynamicCB = GetDX12().dynamicCB.get();

			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::CameraVS), dynamicCB->Allocate(vsCB));
			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::SkinningInputVS), dynamicCB->Allocate(per_object_CB));
			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::MaterialBuffer), dynamicCB->Allocate(psMaterialCB));
			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::CameraPS), dynamicCB->Allocate(psCameraCB));

			if (renderComponent.hasAnimation)
			{
				cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::SkinningOutput), renderComponent.skinningOutData.skinningGpuSrvHandleFinalTransform);
			}
		}

		if (renderComponent.hasTextures)
		{
			scene->GetMaterialManager()->Bindtextures(renderComponent.material.get(), GetDX12().GetCmdList(), static_cast<UINT>(RootSlot::Raster::MaterialTexPS));
		}
		renderComponent.mesh->DrawIndexed(GetDX12().GetCmdList());
	}

	void RenderingManager::CullAndBucketEntities(Scene* scene, const Frustum& frustum)
	{
		individualDraws.clear();

		for (auto& [key, entry] : multiBatches)
		{
			entry.worldMatrices.clear();
		}

		auto group = scene->GetRegistry().group<TransformComponent, RenderComponent>();

		for (auto [entity, transformComponent, renderComponent] : group.each())
		{
			auto aabb = GetWorldAABB(&transformComponent, &renderComponent);
			if (!IsAABBInFrustum(aabb, frustum))
				continue;

			if (renderComponent.hasAnimation)
			{
				individualDraws.push_back(entity);
				continue;
			}

			GpuMesh* meshPtr = renderComponent.mesh.get();
			Material* matPtr = renderComponent.material.get();

			BatchKey key{ meshPtr, matPtr };
			auto& entry = multiBatches[key]; // Creates entry if new

			if (entry.representative == entt::null)
				entry.representative = entity;

			DirectX::XMFLOAT4X4 transposed = MatrixToFloat4x4(DirectX::XMMatrixTranspose(transformComponent.worldMatrix));
			entry.worldMatrices.push_back(transposed);
		}
	}

	void RenderingManager::RenderGbufferInstanced(Scene* scene, const Frustum& frustum)
	{
		if (!scene) return;

		auto* cmdList = GetDX12().GetCmdList();
		auto* dynamicCB = GetDX12().dynamicCB.get();
		if (!dynamicCB || !cmdList) return;

		cmdList->SetGraphicsRootSignature(GetDX12().GetRasterRootSignature());
		cmdList->SetPipelineState(GetDX12().pipelineState_instanced_Gbuffer.Get());

		CB_VS_SimpleShader vsCB = {};
		vsCB.projectionMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(scene->GetCamera().GetProjectionMatrix()));
		vsCB.viewMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(scene->GetCamera().GetViewMatrix()));

		CB_Shader_Camera psCameraCB = {};
		psCameraCB.cameraPos = scene->GetCamera().pos;
		psCameraCB.padding1 = 0.0f;

		CB_VS_Per_Object_Shader per_object_CB = {};
		per_object_CB.HasAnim = FALSE;
		per_object_CB.vertexCount = 0;

		cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::CameraVS), dynamicCB->Allocate(vsCB));
		cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::CameraPS), dynamicCB->Allocate(psCameraCB));
		cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::SkinningInputVS), dynamicCB->Allocate(per_object_CB));

		auto group = scene->GetRegistry().group<TransformComponent, RenderComponent>();

		for (auto& [key, entry] : multiBatches)
		{
			UINT instanceCount = static_cast<UINT>(entry.worldMatrices.size());
			if (instanceCount == 0) continue;

			auto& renderComponent = group.get<RenderComponent>(entry.representative);

			CB_PS_Material psMaterialCB = {};
			psMaterialCB.color = key.mat->baseColor;
			psMaterialCB.hasTextures = renderComponent.hasTextures;
			psMaterialCB.metalness = key.mat->metalness;
			psMaterialCB.roughness = key.mat->roughness;
			psMaterialCB.useAlbedo = key.mat->useAlbedoMap;
			psMaterialCB.useNormals = key.mat->useNormalMap;
			psMaterialCB.useRoughnessMetal = key.mat->useMetalRoughnessMap;
			psMaterialCB.padding = 0.0f;

			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::MaterialBuffer), dynamicCB->Allocate(psMaterialCB));

			MaterialManager* materialMgr = scene->GetMaterialManager();
			if (renderComponent.hasTextures && key.mat)
			{
				materialMgr->Bindtextures(key.mat, cmdList, static_cast<UINT>(RootSlot::Raster::MaterialTexPS));
			}
			else
			{
				auto defaultMat = materialMgr->GetMaterial("DefaultMaterial");
				if (defaultMat)
				{
					materialMgr->Bindtextures(defaultMat.get(), cmdList, static_cast<UINT>(RootSlot::Raster::MaterialTexPS));
				}
			}

			D3D12_GPU_VIRTUAL_ADDRESS matrixBufferGPU = dynamicCB->AllocateArray(entry.worldMatrices.data(), instanceCount);
			cmdList->SetGraphicsRootShaderResourceView(static_cast<UINT>(RootSlot::Raster::InstanceDataBuffer), matrixBufferGPU);

			key.mesh->DrawIndexedInstanced(cmdList, instanceCount);
		}
	}

	void RenderingManager::RenderBRDF()
	{
		ID3D12DescriptorHeap* heaps[] = { GetDX12().GetSharedSrvHeap() };
		GetDX12().GetCmdList()->SetDescriptorHeaps(1, heaps);
		GetDX12().GetCmdList()->SetPipelineState(GetDX12().pipelineState_Brdf.Get());


		float aspect = static_cast<float>(m_brdfMap->m_width) / static_cast<float>(m_brdfMap->m_height);
		float nearZ = 0.01f;
		float farZ = 1000.0f;
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspect, nearZ, farZ);
		DirectX::XMVECTOR position = DirectX::XMVectorZero();
		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(m_brdfMap->m_width);
		viewport.Height = static_cast<float>(m_brdfMap->m_height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = m_brdfMap->m_width;
		scissorRect.bottom = m_brdfMap->m_height;

		GetDX12().GetCmdList()->RSSetViewports(1, &viewport);
		GetDX12().GetCmdList()->RSSetScissorRects(1, &scissorRect);


		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDX12().dsvHeap->GetCPUDescriptorHandleForHeapStart();

		GetDX12().GetCmdList()->OMSetRenderTargets(1, &m_brdfMap->m_rtvHandles[0], FALSE, &dsvHandle);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GetDX12().GetCmdList()->ClearRenderTargetView(m_brdfMap->m_rtvHandles[0], clearColor, 0, nullptr);
		GetDX12().GetCmdList()->ClearDepthStencilView(
			dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,
			0,
			0,
			nullptr
		);

		GetDX12().GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GetDX12().GetCmdList()->IASetVertexBuffers(0, 0, nullptr);
		GetDX12().GetCmdList()->DrawInstanced(3, 1, 0, 0);

		m_brdfMap->TransitionToSRV(GetDX12().GetCmdList());
	}

	void RenderingManager::RayTracedShadows(Scene* scene)
	{
		CB_SHADER_LIGHTS lights_data = {};

		auto* cmdList = GetDX12().GetCmdList();
		auto* dynamicCB = GetDX12().dynamicCB.get();

		m_gBuffer.GetGbufferRenderTargetTexture()->TransitionState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_shadowsTexture->GetResource()->TransitionState(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		cmdList->SetComputeRootSignature(GetDX12().GetGlobalRaytracingRootSignature());
		cmdList->SetPipelineState1(GetDX12().GetRayTracedShadowsResources().rtpso.Get());

		auto lightsView = scene->GetRegistry().view<LightComponent>();
		std::size_t totalLights = lightsView.size();
		lights_data.totalLights = totalLights;
		lights_data.padding3 = DirectX::XMFLOAT3(0, 0, 0);

		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::GbufferTex), m_gBuffer.GetGbufferRenderTargetTexture()->GetSrvGpuHandle(0));
		cmdList->SetComputeRootShaderResourceView(static_cast<UINT>(RootSlot::RayTracing::TlasSRV), m_tlasBuilder.m_tlasBuffer->GetGPUVirtualAddress());
		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::RtUAVOutput), m_shadowsTexture->GetGPUHandleUAV());
		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::LightsStructuredBuffer), scene->GetLightManager()->GetGPUHandle());

		if (dynamicCB)
		{
			cmdList->SetComputeRootConstantBufferView(static_cast<UINT>(RootSlot::RayTracing::GlobalLightData), dynamicCB->Allocate(lights_data));
		}

		GetDX12().DispatchRaytracing(GetDX12().rtShadows_dispatchDesc);

		m_shadowsTexture->GetResource()->TransitionState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void RenderingManager::RayTracedReflections(Scene* scene)
	{
		CB_Shader_Camera psCameraCB = {};
		CB_RT_MeshData rtMeshData = {};

		auto* cmdList = GetDX12().GetCmdList();
		auto* dynamicCB = GetDX12().dynamicCB.get();

		m_gBuffer.GetGbufferRenderTargetTexture()->TransitionState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_reflectionsTexture->GetResource()->TransitionState(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		cmdList->SetComputeRootSignature(GetDX12().GetGlobalRaytracingRootSignature());
		cmdList->SetPipelineState1(GetDX12().GetRayTracedReflectionsResources().rtpso.Get());

		psCameraCB.cameraPos = scene->GetCamera().pos;
		psCameraCB.padding1 = 0.0f;

		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::GbufferTex), m_gBuffer.GetGbufferRenderTargetTexture()->GetSrvGpuHandle(0));
		cmdList->SetComputeRootShaderResourceView(static_cast<UINT>(RootSlot::RayTracing::TlasSRV), m_tlasBuilder.m_tlasBuffer->GetGPUVirtualAddress());
		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::RtUAVOutput), m_reflectionsTexture->GetGPUHandleUAV());
		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::LightsStructuredBuffer), scene->GetLightManager()->GetGPUHandle());

		if (dynamicCB)
		{
			cmdList->SetComputeRootConstantBufferView(static_cast<UINT>(RootSlot::RayTracing::CameraData), dynamicCB->Allocate(psCameraCB));
		}

		rtMeshData.totalVertices = rt_vertexData.size();
		rtMeshData.totalIndices = rt_indexData.size();
		rtMeshData.totalEntities = m_totalEntities;

		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::BindlessAlbedoTextures), m_bindlessAlbedoTextures->GetGPUHandle());
		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::RtVertexData), m_rtEntityHandle->gpuVertexHandle);
		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::RtIndexData), m_rtEntityHandle->gpuIndexHandle);
		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::RtMeshDataOffsets), m_rtEntityHandle->gpuOffsetsHandle);

		if (dynamicCB)
		{
			cmdList->SetComputeRootConstantBufferView(static_cast<UINT>(RootSlot::RayTracing::RtReflectionParams), dynamicCB->Allocate(rtMeshData));
		}

		GetDX12().DispatchRaytracing(GetDX12().rtReflections_dispatchDesc);

		m_reflectionsTexture->GetResource()->TransitionState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void RenderingManager::RayTracedAO(Scene* scene)
	{
		auto* cmdList = GetDX12().GetCmdList();
		auto* dynamicCB = GetDX12().dynamicCB.get();

		m_gBuffer.GetGbufferRenderTargetTexture()->TransitionState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_AOTexture->GetResource()->TransitionState(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		cmdList->SetComputeRootSignature(GetDX12().GetGlobalRaytracingRootSignature());
		cmdList->SetPipelineState1(GetDX12().GetRayTracedAOResources().rtpso.Get());

		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::GbufferTex), m_gBuffer.GetGbufferRenderTargetTexture()->GetSrvGpuHandle(0));
		cmdList->SetComputeRootShaderResourceView(static_cast<UINT>(RootSlot::RayTracing::TlasSRV), m_tlasBuilder.m_tlasBuffer->GetGPUVirtualAddress());
		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::RtUAVOutput), m_AOTexture->GetGPUHandleUAV());

		if (dynamicCB)
		{
			cmdList->SetComputeRootConstantBufferView(static_cast<UINT>(RootSlot::RayTracing::RtAoParams), dynamicCB->Allocate(aoData));
		}

		GetDX12().DispatchRaytracing(GetDX12().rtAO_dispatchDesc);

		m_AOTexture->GetResource()->TransitionState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void RenderingManager::DispatchRays(Scene* scene)
	{
		RefitBLAS(scene);
		BuildTLAS(scene);

		RayTracedShadows(scene);
		RayTracedReflections(scene);
		RayTracedAO(scene);	
	}

	void RenderingManager::RenderLightPass(Scene* scene)
	{
		CB_PS_PBR cb_ps_pbr = {};
		CB_Shader_Camera psCameraCB = {};

		auto* cmdList = GetDX12().GetCmdList();
		auto* dynamicCB = GetDX12().dynamicCB.get();

		m_gBuffer.GetGbufferRenderTargetTexture()->TransitionState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		CB_SHADER_LIGHTS lights_data = {};

		ID3D12DescriptorHeap* heaps[] = { GetDX12().GetSharedSrvHeap() };
		cmdList->SetDescriptorHeaps(1, heaps);

		cb_ps_pbr.mip_roughness = 0.0f;
		cb_ps_pbr.ambientColor = GetAmbientColor();
		cb_ps_pbr.exposureGamma = DirectX::XMFLOAT4(GetExposure(), GetGamma(), 0.0f, 0.0f);

		psCameraCB.cameraPos = scene->GetCamera().pos;
		psCameraCB.screenSize = DirectX::XMFLOAT2(static_cast<float>(GetDX12().GetScreenWidth()), static_cast<float>(GetDX12().GetScreenHeight()));
		psCameraCB.padding1 = 0.0f;
		psCameraCB.padding2 = 0.0f;

		m_brdfMap->TransitionState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::GbufferTexPS), m_gBuffer.GetGbufferRenderTargetTexture()->GetSrvGpuHandle(0));
		cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::CameraPS), dynamicCB->Allocate(psCameraCB));
		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::PrefilterMap), m_prefilterMap.GetCubeMapRenderTargetTexture()->GetSrvGpuHandle(0));
		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::IrradianceMap), m_irradianceMap.GetCubeMapRenderTargetTexture()->GetSrvGpuHandle(0));
		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::BrdfLUT), m_brdfMap->GetSrvGpuHandle(0));
		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::RtShadowsInput), m_shadowsTexture->GetGPUHandle());
		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::ShadowsData), scene->GetLightManager()->GetShadowsSrvGPUHandle());
		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::RtReflections), m_reflectionsTexture->GetGPUHandle());
		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::RtAmbientOccl), m_AOTexture->GetGPUHandle());

		// Get all the light components in the scene
		auto lightsView = scene->GetRegistry().view<LightComponent>();
		std::size_t totalLights = lightsView.size();
		lights_data.totalLights = totalLights;
		lights_data.padding3 = DirectX::XMFLOAT3(0, 0, 0);

		if (dynamicCB)
		{
			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::PbrParamsPS), dynamicCB->Allocate(cb_ps_pbr));
			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::GlobalLightData), dynamicCB->Allocate(lights_data));
		}

		float aspect = static_cast<float>(m_lightPassRenderTarget->m_width) / static_cast<float>(m_lightPassRenderTarget->m_height);
		float nearZ = 0.01f;
		float farZ = 1000.0f;
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspect, nearZ, farZ);
		DirectX::XMVECTOR position = DirectX::XMVectorZero();

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(m_lightPassRenderTarget->m_width);
		viewport.Height = static_cast<float>(m_lightPassRenderTarget->m_height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = m_lightPassRenderTarget->m_width;
		scissorRect.bottom = m_lightPassRenderTarget->m_height;

		cmdList->RSSetViewports(1, &viewport);
		cmdList->RSSetScissorRects(1, &scissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDX12().dsvHeap->GetCPUDescriptorHandleForHeapStart();

		cmdList->OMSetRenderTargets(1, &m_lightPassRenderTarget->m_rtvHandles[0], FALSE, &dsvHandle);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		cmdList->ClearRenderTargetView(m_lightPassRenderTarget->m_rtvHandles[0], clearColor, 0, nullptr);
		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		cmdList->SetPipelineState(GetDX12().pipelineState_2D.Get());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->IASetVertexBuffers(0, 0, nullptr);
		cmdList->DrawInstanced(3, 1, 0, 0);

		m_cubeMap1.RenderCubeMap(GetDX12(), scene->GetCamera(), m_gBuffer);

		m_lightPassRenderTarget->TransitionToSRV(cmdList);

		FXAA();
	}

	void RenderingManager::CalculateCompute(Scene* scene)
	{
		m_computeSkinning.Compute(scene);
	}

	void RenderingManager::UpdatePBR(Scene* scene)
	{
		// Render cube maps, irradiance, prefilter and brdf maps
		RenderPbrMaps(scene->GetCamera());

		// Render light pass
		RenderLightPass(scene);
	}

	void RenderingManager::DebugDraw(Scene* scene)
	{
		scene->GetPhysicsManager()->GetDebugDraw()->DebugDraw(GetDX12(), scene->GetCamera());
	}

	void RenderingManager::FXAA()
	{
		auto* cmdList = GetDX12().GetCmdList();

		fxaaCB.invScreenSize = DirectX::XMFLOAT2(
			1.0f / static_cast<float>(GetDX12().GetScreenWidth()),
			1.0f / static_cast<float>(GetDX12().GetScreenHeight())
		);

		ID3D12DescriptorHeap* heaps[] = { GetDX12().GetSharedSrvHeap() };
		cmdList->SetDescriptorHeaps(1, heaps);

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<float>(GetDX12().GetScreenWidth());
		viewport.Height = static_cast<float>(GetDX12().GetScreenHeight());
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = GetDX12().GetScreenWidth();
		scissorRect.bottom = GetDX12().GetScreenHeight();

		cmdList->RSSetViewports(1, &viewport);
		cmdList->RSSetScissorRects(1, &scissorRect);

		CD3DX12_CPU_DESCRIPTOR_HANDLE backbufferRTV(
			GetDX12().GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(),
			GetDX12().frameIndex,
			GetDX12().rtvDescriptorSize
		);

		cmdList->OMSetRenderTargets(1, &backbufferRTV, FALSE, nullptr);

		cmdList->SetPipelineState(GetDX12().pipelineState_FXAA.Get());
		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::LightPassTexPS), m_lightPassRenderTarget->GetSrvGpuHandle(0));

		if(GetDX12().dynamicCB)
			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::FxaaParamsPS), GetDX12().dynamicCB->Allocate(fxaaCB));

		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->IASetVertexBuffers(0, 0, nullptr);
		cmdList->DrawInstanced(3, 1, 0, 0);
	}

	void RenderingManager::SetGbufferRenderTarget()
	{
		// Render the scene to the geometry pass
		float clearColor[] = { 0,0,0,1 };
		SetRenderTarget(*GetGbuffer().GetGbufferRenderTargetTexture(), clearColor);
	}

	DirectX::XMFLOAT3 RenderingManager::GetAmbientColor() const
	{
		return m_ambientColor;
	}
	float RenderingManager::GetExposure() const
	{
		return m_exposure;
	}
	float RenderingManager::GetGamma() const
	{
		return m_gamma;
	}
}