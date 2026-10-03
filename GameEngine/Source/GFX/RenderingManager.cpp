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
		m_bindlessTextures.reset();
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

	// Populate mesh data such as vertex/index buffers, mesh data offsets and bindless textures
	void RenderingManager::PopulateMeshData(Scene* scene)
	{
		m_sharedMeshes.clear();
		m_meshLookup.clear();
		m_meshDataOffsests.clear();
		rt_meshDataOffsests.clear();
		rt_vertexData.clear();
		rt_indexData.clear();
		m_totalEntities = 0;
		m_totalRTentities = 0;

		auto* assetManager = scene->GetAssetManager();
		assetManager->globalVertices.clear();
		assetManager->globalIndices.clear();

		m_bindlessTextures = std::make_unique<Texture12>();

		m_rtEntityHandle = std::make_unique <RTEntityHandle>();
		m_meshDataOffsetsHandle = std::make_unique<MeshDataOffsetsHandle>();
		m_instanceDataHandle = std::make_unique<InstanceDataHandle>();
		m_indirectCommandHandle = std::make_unique<IndirectCommandHandle>();

		m_texturesMapping = std::make_unique<std::map<uint32_t, std::shared_ptr<Texture12>>>();

		UINT textureIndex = 0;
		auto* MaterialManager = scene->GetMaterialManager();
		for (const auto& [id, material] : MaterialManager->m_materials)
		{
			
			material->albedoIndex = textureIndex;
			m_texturesMapping->emplace(textureIndex++, material->albedoTexture);
		
			if (material->normalTexture)
			{
				material->normalIndex = textureIndex;
				m_texturesMapping->emplace(textureIndex++, material->normalTexture);
			}
			if (material->metalRoughnessTexture)
			{
				material->metalRoughnessIndex = textureIndex;
				m_texturesMapping->emplace(textureIndex++, material->metalRoughnessTexture);
			}
		}
		
		uint32_t vertexSize = 0, indicesSize = 0;
		uint32_t rtVertexSize = 0, rtIndicesSize = 0;

		auto group = scene->GetRegistry().group<>(entt::get<ECS::TransformComponent, ECS::RenderComponent>);
		for (auto [entity, transformComponent, renderComponent] : group.each())
		{
			auto& cpuMesh = renderComponent.mesh->cpuMesh;
			const bool isSkeletal = renderComponent.meshType == ECS::MESH_TYPE::SKELETAL_MESH;

			uint32_t geoVertexOffset = 0, geoIndexOffset = 0;

			if (!isSkeletal)
			{
				const void* key = cpuMesh.get();
				auto it = m_meshLookup.find(key);
				uint32_t id;

				if (it == m_meshLookup.end())
				{
					id = static_cast<uint32_t>(m_sharedMeshes.size());
					m_sharedMeshes.push_back({ vertexSize, indicesSize, static_cast<uint32_t>(cpuMesh->indices.size()) });
					m_meshLookup.emplace(key, id);

					assetManager->globalVertices.insert(assetManager->globalVertices.end(), cpuMesh->vertices.begin(), cpuMesh->vertices.end());
					assetManager->globalIndices.insert(assetManager->globalIndices.end(), cpuMesh->indices.begin(), cpuMesh->indices.end());

					vertexSize += static_cast<uint32_t>(cpuMesh->vertices.size());
					indicesSize += static_cast<uint32_t>(cpuMesh->indices.size());
				}
				else
					id = it->second;

				renderComponent.sharedMeshID = id;
				geoVertexOffset = m_sharedMeshes[id].vertexOffset;
				geoIndexOffset = m_sharedMeshes[id].indexOffset;
			}

			ECS::MeshDataOffsets dataOffsets = {};
			dataOffsets.vertexOffset = geoVertexOffset;     // shared offsets for instancing
			dataOffsets.indexOffset = geoIndexOffset;
			dataOffsets.albedoIndex = renderComponent.material->albedoIndex;
			dataOffsets.normalIndex = renderComponent.material->normalIndex;
			dataOffsets.metalRoughnessIndex = renderComponent.material->metalRoughnessIndex;
			dataOffsets.hasTextures = (renderComponent.meshType != ECS::MESH_TYPE::LIGHT);

			renderComponent.meshDataIndex = static_cast<UINT>(m_meshDataOffsests.size());
			m_meshDataOffsests.push_back(dataOffsets);
			m_totalEntities++;

			// Raytracing data population for static meshes and skeletal meshes
			if (renderComponent.meshType == ECS::STATIC_MESH || isSkeletal)
			{
				ECS::MeshDataOffsets rtDataOffsets = {};
				rtDataOffsets.vertexOffset = rtVertexSize;
				rtDataOffsets.indexOffset = rtIndicesSize;
				rtDataOffsets.albedoIndex = renderComponent.material->albedoIndex;
				rtDataOffsets.normalIndex = renderComponent.material->normalIndex;
				rtDataOffsets.metalRoughnessIndex = renderComponent.material->metalRoughnessIndex;
				rtDataOffsets.hasTextures = renderComponent.material->hasTextures;
				rt_meshDataOffsests.push_back(rtDataOffsets);

				for (const auto& v : cpuMesh->vertices)
				{
					ECS::VertexData d;
					d.position = v.pos; d.uv = v.texCoord;
					d.padding1 = 1.0f;  d.padding2 = { 1.0f, 1.0f };
					rt_vertexData.push_back(d);
				}
				for (auto idx : cpuMesh->indices)
				{
					ECS::IndexData d; d.indices = idx;
					rt_indexData.push_back(d);
				}
				rtVertexSize += static_cast<uint32_t>(cpuMesh->vertices.size());
				rtIndicesSize += static_cast<uint32_t>(cpuMesh->indices.size());
			}
			m_totalRTentities++;
		}

		m_perMeshInstances.assign(m_sharedMeshes.size(), {});

		// Buffers for indirect drawing
		assetManager->UploadGlobalBuffers(GetDX12().GetDevice(), GetDX12().GetCmdList());

		if (!m_texturesMapping->empty())
		{
			m_bindlessTextures->CreateBindlessTexture(GetDX12().GetDevice(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), m_texturesMapping.get());

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


			// For the mesh data offsets, we need to create a separate handle for the ray tracing pipeline
			m_rtEntityHandle->rtMeshDataOffsets.Initialize(GetDX12().GetDevice(), m_totalRTentities);
			allocator = GetDX12().GetDescriptorAllocator()->Allocate();
			m_rtEntityHandle->cpuOffsetsHandle = allocator.cpuHandle;
			m_rtEntityHandle->gpuOffsetsHandle = allocator.gpuHandle;
			m_rtEntityHandle->rtMeshDataOffsets.CreateSRV(GetDX12().GetDevice(), m_rtEntityHandle->cpuOffsetsHandle);

			m_rtEntityHandle->rtVertexGpuData.UploadData(GetDX12().GetCmdList(), rt_vertexData);
			m_rtEntityHandle->rtIndexGpuData.UploadData(GetDX12().GetCmdList(), rt_indexData);
			m_rtEntityHandle->rtMeshDataOffsets.UploadData(GetDX12().GetCmdList(), rt_meshDataOffsests);


			// For the mesh data offsets, we need to create a separate handle for the rasterization pipeline
			m_meshDataOffsetsHandle->meshDataOffsets.Initialize(GetDX12().GetDevice(), m_totalEntities);

			allocator = GetDX12().GetDescriptorAllocator()->Allocate();
			m_meshDataOffsetsHandle->cpuOffsetsHandle = allocator.cpuHandle;
			m_meshDataOffsetsHandle->gpuOffsetsHandle = allocator.gpuHandle;
			m_meshDataOffsetsHandle->meshDataOffsets.CreateSRV(GetDX12().GetDevice(), m_meshDataOffsetsHandle->cpuOffsetsHandle);
			m_meshDataOffsetsHandle->meshDataOffsets.UploadData(GetDX12().GetCmdList(), m_meshDataOffsests);

			m_instanceDataHandle->instanceData.Initialize(GetDX12().GetDevice(), m_totalEntities);
			m_instanceDataHandle->instanceCount = m_totalEntities;
			allocator = GetDX12().GetDescriptorAllocator()->Allocate();
			m_instanceDataHandle->cpuInstanceHandle = allocator.cpuHandle;
			m_instanceDataHandle->gpuInstanceHandle = allocator.gpuHandle;
			m_instanceDataHandle->instanceData.CreateSRV(GetDX12().GetDevice(), m_instanceDataHandle->cpuInstanceHandle);

			m_indirectCommandHandle->indirectCommands.Initialize(GetDX12().GetDevice(), m_totalEntities);
			allocator = GetDX12().GetDescriptorAllocator()->Allocate();
			m_indirectCommandHandle->cpuIndirectCommandHandle = allocator.cpuHandle;
			m_indirectCommandHandle->gpuIndirectCommandHandle = allocator.gpuHandle;
			m_indirectCommandHandle->indirectCommands.CreateSRV(GetDX12().GetDevice(), m_indirectCommandHandle->cpuIndirectCommandHandle);

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

		GetDX12().GetCmdList()->SetPipelineState(GetDX12().pipelineState_Gbuffer.Get());
		vsCB.projectionMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(scene->GetCamera().GetProjectionMatrix()));
		vsCB.viewMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(scene->GetCamera().GetViewMatrix()));
		vsCB.worldMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(transformComponent.worldMatrix));

		auto& cpuMesh = renderComponent.mesh->cpuMesh;
		std::size_t vertexCount = cpuMesh->vertices.size();
		per_object_CB.worldMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(transformComponent.worldMatrix));
		per_object_CB.vertexCount = vertexCount;
		per_object_CB.HasAnim = renderComponent.hasAnimation;
		per_object_CB.meshDataIndex = renderComponent.meshDataIndex;

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
		psMaterialCB.meshDataIndex = renderComponent.meshDataIndex;
		psMaterialCB.bDrawIndirect = false;

		psMaterialCB.padding[0] = 0;
		psMaterialCB.padding[1] = 0;

		if (GetDX12().dynamicCB)
		{
			auto* cmdList = GetDX12().GetCmdList();
			auto* dynamicCB = GetDX12().dynamicCB.get();

			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::CameraVS), dynamicCB->Allocate(vsCB));
			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::SkinningInputVS), dynamicCB->Allocate(per_object_CB));
			cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::MaterialBuffer), dynamicCB->Allocate(psMaterialCB));


			if (renderComponent.hasAnimation)
			{
				cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::SkinningOutput), renderComponent.skinningOutData.skinningGpuSrvHandleFinalTransform);
			}
		}

		renderComponent.mesh->DrawIndexed(GetDX12().GetCmdList());
	}

	void RenderingManager::RenderGbufferIndirect(Scene* scene, const std::vector<IndirectCommand>& indirectCommands, const std::vector<GBufferInstanceData>& gbufferInstanceData)
	{
		if (indirectCommands.empty())
			return;

		auto* assetManager = scene->GetAssetManager();

		if (gbufferInstanceData.size() > m_instanceDataHandle->instanceCount)
		{
			m_instanceDataHandle->instanceCount = static_cast<uint32_t>(gbufferInstanceData.size() * 1.5f);

			m_instanceDataHandle->instanceData.Initialize(GetDX12().GetDevice(), m_instanceDataHandle->instanceCount);
			m_instanceDataHandle->instanceData.CreateSRV(GetDX12().GetDevice(), m_instanceDataHandle->cpuInstanceHandle);

			m_indirectCommandHandle->indirectCommands.Initialize(GetDX12().GetDevice(), m_instanceDataHandle->instanceCount);
			m_indirectCommandHandle->indirectCommands.CreateSRV(GetDX12().GetDevice(), m_indirectCommandHandle->cpuIndirectCommandHandle);
		}

		auto* cmdList = GetDX12().GetCmdList();
	
		cmdList->SetPipelineState(GetDX12().pipelineState_instanced_Gbuffer.Get());
		cmdList->SetGraphicsRootSignature(GetDX12().GetRasterRootSignature()); // REQUIRED

		ID3D12DescriptorHeap* heaps[] = { GetDX12().GetSharedSrvHeap() };
		GetDX12().GetCmdList()->SetDescriptorHeaps(1, heaps);


		m_instanceDataHandle->instanceData.UploadData(cmdList, gbufferInstanceData);
		m_indirectCommandHandle->indirectCommands.UploadData(cmdList, indirectCommands);

		D3D12_RESOURCE_BARRIER barriers[2] = {};

		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
			m_instanceDataHandle->instanceData.GetResource()->GetResource(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		);

		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
			m_indirectCommandHandle->indirectCommands.GetResource()->GetResource(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT
		);

		cmdList->ResourceBarrier(2, barriers);

		m_instanceDataHandle->instanceData.GetResource()->SetCurrentState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_indirectCommandHandle->indirectCommands.GetResource()->SetCurrentState(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

		cmdList->SetGraphicsRootDescriptorTable(
			static_cast<UINT>(RootSlot::Raster::InstanceDataBuffer),
			m_instanceDataHandle->gpuInstanceHandle
		);
		cmdList->SetGraphicsRootDescriptorTable(
			static_cast<UINT>(RootSlot::Raster::MeshDataOffsets),
			m_meshDataOffsetsHandle->gpuOffsetsHandle
		);

		UpdateBuffers(scene);

		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->IASetVertexBuffers(0, 1, assetManager->globalVertexBuffer.GetBufferViewPtr());
		cmdList->IASetIndexBuffer(assetManager->globalIndexBuffer.GetBufferViewPtr());

		cmdList->ExecuteIndirect(
			GetDX12().GetCommandSignature(),
			static_cast<UINT>(indirectCommands.size()),
			m_indirectCommandHandle->indirectCommands.GetResource()->GetResource(),
			0,
			nullptr,
			0
		);
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
		cmdList->SetComputeRootDescriptorTable(static_cast<UINT>(RootSlot::RayTracing::BindlessTextures), m_bindlessTextures->GetGPUHandle());

		if (dynamicCB)
		{
			cmdList->SetComputeRootConstantBufferView(static_cast<UINT>(RootSlot::RayTracing::CameraData), dynamicCB->Allocate(psCameraCB));
		}

		rtMeshData.totalVertices = rt_vertexData.size();
		rtMeshData.totalIndices = rt_indexData.size();
		rtMeshData.totalEntities = m_totalRTentities;

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

	void RenderingManager::BuildIndirectDraws(Scene* scene, const Frustum& frustum, std::vector<IndirectCommand>& outCommands, std::vector<GBufferInstanceData>& outInstances)
	{
		outCommands.clear();
		outInstances.clear();
		for (auto& v : m_perMeshInstances) v.clear();

		auto group = scene->GetRegistry().group<TransformComponent, RenderComponent>();

		for (auto [entity, transformComponent, renderComponent] : group.each())
		{
			ECS::AABB worldAABB = ComputeWorldAABB(transformComponent.aabb, transformComponent.worldMatrix);
			if (!IsAABBInFrustum(worldAABB, frustum))
				continue;

			if (renderComponent.meshType == MESH_TYPE::SKELETAL_MESH)
			{
				RenderGbuffer(scene, entity, transformComponent, renderComponent);
				continue;
			}

			GBufferInstanceData inst = {};
			inst.worldMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(transformComponent.worldMatrix));
			inst.meshIndex = renderComponent.meshDataIndex;
			m_perMeshInstances[renderComponent.sharedMeshID].push_back(inst);
		}

		uint32_t firstInstance = 0;
		for (size_t meshID = 0; meshID < m_perMeshInstances.size(); ++meshID)
		{
			auto& list = m_perMeshInstances[meshID];
			if (list.empty()) continue;

			const auto& m = m_sharedMeshes[meshID];
			IndirectCommand cmd = {};
			cmd.firstInstance = firstInstance;
			cmd.drawArgs.IndexCountPerInstance = m.indexCount;
			cmd.drawArgs.InstanceCount = static_cast<UINT>(list.size());
			cmd.drawArgs.StartIndexLocation = m.indexOffset;
			cmd.drawArgs.BaseVertexLocation = static_cast<INT>(m.vertexOffset);
			cmd.drawArgs.StartInstanceLocation = 0;
			outCommands.push_back(cmd);

			outInstances.insert(outInstances.end(), list.begin(), list.end());
			firstInstance += static_cast<uint32_t>(list.size());
		}
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

	void RenderingManager::UpdateBuffers(Scene* scene)
	{
		auto* cmdList = GetDX12().GetCmdList();
		auto* dynamicCB = GetDX12().dynamicCB.get();

		cmdList->SetGraphicsRootSignature(GetDX12().GetRasterRootSignature());

		CB_VS_SimpleShader vsCameraCB = {};
		vsCameraCB.projectionMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(scene->GetCamera().GetProjectionMatrix()));
		vsCameraCB.viewMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(scene->GetCamera().GetViewMatrix()));

		cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::CameraVS), dynamicCB->Allocate(vsCameraCB));

		CB_Shader_Camera psCameraCB = {};
		psCameraCB.cameraPos = scene->GetCamera().pos;
		psCameraCB.padding1 = 0.0f;
		cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::CameraPS), dynamicCB->Allocate(psCameraCB));

		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::BindlessTextures), m_bindlessTextures->GetGPUHandle());
		cmdList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Raster::MeshDataOffsets), m_meshDataOffsetsHandle->gpuOffsetsHandle);

		CB_PS_Material psMaterialCB = {};

		psMaterialCB.bDrawIndirect = true;

		cmdList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Raster::MaterialBuffer), dynamicCB->Allocate(psMaterialCB));
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