#include "RenderingManager.h"
#include "ConstantBufferTypes.h"
#include "Scene.h"
#include "ErrorLogger.h"
#include "GameWindow.h"
#include <cassert>
#include "BLASBuilder.h"
#include "AssetManager.h"
#include "Physics/PhysicsManager.h"
#include "MathHelpers.h"

namespace ECS
{
	RenderingManager::RenderingManager()
	{
		m_ambientColor = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
		m_exposure = 0.5f;
		m_gamma = 2.2f;
	}

	RenderingManager::~RenderingManager()
	{
		m_textureUAV.reset();
		m_shadowsUAV.reset();
	}

	bool RenderingManager::Initialize(GameWindow& game_window, int width, int height)
	{
	
		GetDX12().Initialize(game_window.GetWindow(), width, height);
		if (!m_gui.Initialize(game_window.GetSDLWindow(), GetDX12().GetDevice(), GetDX12().GetCommandQueue(), GetDX12().GetRtvHeap(), GetDX12().GetDescriptorAllocator()))
		{
			ErrorLogger::Log("Failed to initialize ImGui!");
			return false;
		}
		
		m_computeSkinning.Initialize();
		return true;
	}

	void RenderingManager::InitializeRenderTargets(int& width, int& height)
	{
		hdr_map1.Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetDescriptorAllocator(), "Data/HDR/warm_restaurant_night_2k.hdr");

		m_gBuffer.Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), width, height);
		m_cubeMap1.Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), 512, 512);
		m_irradianceMap.Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), 64, 64);
		m_prefilterMap.Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), 512, 512, 5);

		std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
		m_brdfMap = std::make_unique<RenderTargetTexture>(1);
		m_brdfMap->Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), 512, 512, formats, 1);


		m_textureUAV = std::make_unique<Texture12>();
		m_textureUAV->CreateTextureUAV(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetDescriptorAllocator(), width, height);
		m_shadowsUAV = std::make_unique<Texture12>();
		m_shadowsUAV->CreateTextureUAV(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetDescriptorAllocator(), width, height);

		m_raytracingMap = std::make_unique<RenderTargetTexture>(1);
		m_raytracingMap->Initialize(GetDX12().GetDevice(), GetDX12().GetCmdList(), GetDX12().GetCommandAllocator(), GetDX12().GetSharedSrvHeap(), GetDX12().GetDescriptorAllocator(), width, height, formats, 1);
	}

	void RenderingManager::BuildTLAS(Scene* scene)
	{
		// Build TLAS for raytracing
		m_tlasBuilder.Build(scene);
	}

	void RenderingManager::ReBuildBLAS(Scene* scene)
	{
		BLASBuilder blas_builder;
		auto group = scene->GetRegistry().group<>(entt::get<RenderComponent, AnimatorComponent>);

		for (auto entity : group)
		{
			auto& renderComponent = group.get<RenderComponent>(entity);

			if (renderComponent.meshType == SKELETAL_MESH)
			{
				blas_builder.ReBuild(GetDX12().GetDevice(), GetDX12().GetCmdList(), renderComponent);
			}
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
		m_raytracingMap->Reset(GetDX12().GetCmdList());
	}

	void RenderingManager::SetRenderTarget(RenderTargetTexture& renderTarget, float* clearColor)
	{
		renderTarget.SetRenderTarget(GetDX12().GetCmdList(), GetDX12().dsvHandle, clearColor);
	}

	void RenderingManager::RenderPbrMaps(Camera& camera)
	{
		if (bRenderPbrPass)
		{
			m_cubeMap1.Render(GetDX12(), camera, GetDX12().pipelineState_Cubemap.Get(), 8, hdr_map1.GetHDRtexture()->GetGPUHandle());
			// Render the irradiance map
			m_irradianceMap.Render(GetDX12(), camera, GetDX12().pipelineState_IrradianceConv.Get(), 9, m_cubeMap1.GetCubeMapRenderTargetTexture()->GetSrvGpuHandle(0));
			// Render the prefilter map
			m_prefilterMap.RenderMips(GetDX12(), camera, GetDX12().pipelineState_Prefilter.Get(), 9, m_cubeMap1.GetCubeMapRenderTargetTexture()->GetSrvGpuHandle(0));
			// Render the brdf map
			RenderBRDF();
			bRenderPbrPass = false;
		}
	}

	void RenderingManager::RenderGbuffer(Scene* scene, entt::entity& entity, Camera& camera,
		TransformComponent& transformComponent, RenderComponent& renderComponent)
	{
		if (!scene)
			return;

		CB_VS_SimpleShader vsCB = {};
		CB_VS_AnimationShader skinningCB = {};
		CB_PS_SimpleShader psCB = {};
		CB_PS_Material psMaterialCB = {};
		CB_PS_Camera psCameraCB = {};
		CB_PS_PBR cb_ps_pbr = {};

		GetDX12().GetCmdList()->SetPipelineState(GetDX12().pipelineState_Gbuffer.Get());
		vsCB.projectionMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(camera.GetProjectionMatrix()));
		vsCB.viewMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(camera.GetViewMatrix()));
		vsCB.worldMatrix = MatrixToFloat4x4(DirectX::XMMatrixTranspose(transformComponent.worldMatrix));

		auto& cpuMesh = renderComponent.mesh->cpuMesh;
		std::size_t vertexCount = cpuMesh->vertices.size();
		skinningCB.vertexCount = vertexCount;
		skinningCB.padding = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		skinningCB.HasAnim = renderComponent.hasAnimation;

		psCB.lightPos = DirectX::XMFLOAT4(3.0f, 5.0f, 1.0f, 1.0f);
		psCB.color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

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
		
		psCameraCB.cameraPos = camera.pos;
		psCameraCB.padding1 = 0.0f;

		cb_ps_pbr.mip_roughness = 0.0f;
		cb_ps_pbr.ambientColor = GetAmbientColor();
		cb_ps_pbr.exposureGamma = DirectX::XMFLOAT4(GetExposure(), GetGamma(), 0.0f, 0.0f);

		if (GetDX12().GetCmdList())
		{
			if (GetDX12().dynamicCB)
			{
				GetDX12().GetCmdList()->SetGraphicsRootConstantBufferView(0, GetDX12().dynamicCB->Allocate(vsCB));
				GetDX12().GetCmdList()->SetGraphicsRootConstantBufferView(1, GetDX12().dynamicCB->Allocate(psCB));
				GetDX12().GetCmdList()->SetGraphicsRootConstantBufferView(3, GetDX12().dynamicCB->Allocate(skinningCB));
				GetDX12().GetCmdList()->SetGraphicsRootConstantBufferView(5, GetDX12().dynamicCB->Allocate(psMaterialCB));
				GetDX12().GetCmdList()->SetGraphicsRootConstantBufferView(6, GetDX12().dynamicCB->Allocate(psCameraCB));
				GetDX12().GetCmdList()->SetGraphicsRootConstantBufferView(10, GetDX12().dynamicCB->Allocate(cb_ps_pbr));

				if (renderComponent.hasAnimation)
				{
					GetDX12().GetCmdList()->SetGraphicsRootDescriptorTable(
						19, // Root parameter skinning structured buffer output
						renderComponent.skinningOutData.skinningGpuSrvHandleFinalTransform
					);
				}
			}

			if(renderComponent.hasTextures)
				scene->GetMaterialManager()->Bindtextures(renderComponent.material.get(), GetDX12().GetCmdList(), 2);
			renderComponent.mesh->DrawIndexed(GetDX12().GetCmdList());
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

	void RenderingManager::DispatchRays(Scene* scene)
	{
		CB_SHADER_LIGHTS lights_data = {};

		ReBuildBLAS(scene);
		BuildTLAS(scene);

		GetDX12().CreateSBT(scene->blas_total);

		m_gBuffer.GetGbufferRenderTargetTexture()->TransitionState(GetDX12().GetCmdList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		// Transition back to unorder access
		m_shadowsUAV->GetResource()->TransitionState(GetDX12().GetCmdList(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		GetDX12().GetCmdList()->SetComputeRootSignature(GetDX12().GetGlobalRaytracingRootSignature());
		GetDX12().GetCmdList()->SetPipelineState1(GetDX12().rtpso.Get());

		// Get all the light components in the scene
		auto lightsView = scene->GetRegistry().view<LightComponent>();
		std::size_t totalLights = lightsView.size();
		lights_data.totalLights = totalLights;
		lights_data.padding3 = DirectX::XMFLOAT3(0, 0, 0);

		GetDX12().GetCmdList()->SetComputeRootDescriptorTable(0, m_gBuffer.GetGbufferRenderTargetTexture()->GetSrvGpuHandle(0));
		GetDX12().GetCmdList()->SetComputeRootShaderResourceView(1, m_tlasBuilder.m_tlasBuffer->GetGPUVirtualAddress());
		GetDX12().GetCmdList()->SetComputeRootDescriptorTable(2, m_shadowsUAV->GetGPUHandleUAV());
		GetDX12().GetCmdList()->SetComputeRootDescriptorTable(3, scene->GetLightManager()->GetGPUHandle());
		if (GetDX12().dynamicCB)
		{
			GetDX12().GetCmdList()->SetComputeRootConstantBufferView(4, GetDX12().dynamicCB->Allocate(lights_data));
		}

		GetDX12().DispatchRaytracing();

		// Transition to shader resource
		m_shadowsUAV->GetResource()->TransitionState(GetDX12().GetCmdList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		RenderRayTracingToRenderTarget();
	}

	void RenderingManager::RenderLightPass(Scene* scene)
	{
		m_gBuffer.GetGbufferRenderTargetTexture()->TransitionState(GetDX12().GetCmdList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		CB_SHADER_LIGHTS lights_data = {};

		ID3D12DescriptorHeap* heaps[] = { GetDX12().GetSharedSrvHeap() };
		GetDX12().GetCmdList()->SetDescriptorHeaps(1, heaps);

		float aspect = static_cast<float>(GetDX12().GetScreenWidth()) / static_cast<float>(GetDX12().GetScreenHeight());
		float nearZ = 0.01f;
		float farZ = 1000.0f;
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspect, nearZ, farZ);
		DirectX::XMVECTOR position = DirectX::XMVectorZero();
		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(GetDX12().GetScreenWidth());
		viewport.Height = static_cast<float>(GetDX12().GetScreenHeight());
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = GetDX12().GetScreenWidth();
		scissorRect.bottom = GetDX12().GetScreenHeight();

		GetDX12().GetCmdList()->RSSetViewports(1, &viewport);
		GetDX12().GetCmdList()->RSSetScissorRects(1, &scissorRect);


		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDX12().dsvHeap->GetCPUDescriptorHandleForHeapStart();
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetDX12().GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), GetDX12().frameIndex, GetDX12().rtvDescriptorSize);

		GetDX12().GetCmdList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GetDX12().GetCmdList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		GetDX12().GetCmdList()->ClearDepthStencilView(
			dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,
			0,
			0,
			nullptr
		);


		GetDX12().GetCmdList()->SetGraphicsRootDescriptorTable(4, m_gBuffer.GetGbufferRenderTargetTexture()->GetSrvGpuHandle(0));
		GetDX12().GetCmdList()->SetGraphicsRootDescriptorTable(11, m_prefilterMap.GetCubeMapRenderTargetTexture()->GetSrvGpuHandle(0));
		GetDX12().GetCmdList()->SetGraphicsRootDescriptorTable(12, m_irradianceMap.GetCubeMapRenderTargetTexture()->GetSrvGpuHandle(0));
		GetDX12().GetCmdList()->SetGraphicsRootDescriptorTable(13, m_brdfMap->GetSrvGpuHandle(0));
		GetDX12().GetCmdList()->SetGraphicsRootDescriptorTable(18, m_raytracingMap->GetSrvGpuHandle(0));

		// Get all the light components in the scene
		auto lightsView = scene->GetRegistry().view<LightComponent>();
		std::size_t totalLights = lightsView.size();
		lights_data.totalLights = totalLights;
		lights_data.padding3 = DirectX::XMFLOAT3(0, 0, 0);

		if (GetDX12().dynamicCB)
		{
			GetDX12().GetCmdList()->SetGraphicsRootConstantBufferView(14, GetDX12().dynamicCB->Allocate(lights_data));
		}

		GetDX12().GetCmdList()->SetPipelineState(GetDX12().pipelineState_2D.Get());
		GetDX12().GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GetDX12().GetCmdList()->IASetVertexBuffers(0, 0, nullptr);
		GetDX12().GetCmdList()->DrawInstanced(3, 1, 0, 0);
	}

	void RenderingManager::RenderRayTracingToRenderTarget()
	{

		ID3D12DescriptorHeap* heaps[] = { GetDX12().GetSharedSrvHeap() };
		GetDX12().GetCmdList()->SetDescriptorHeaps(1, heaps);
		GetDX12().GetCmdList()->SetPipelineState(GetDX12().pipelineState_raytracingRenderTarget.Get());

		float aspect = static_cast<float>(m_raytracingMap->m_width) / static_cast<float>(m_raytracingMap->m_height);
		float nearZ = 0.01f;
		float farZ = 1000.0f;
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspect, nearZ, farZ);
		DirectX::XMVECTOR position = DirectX::XMVectorZero();
		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(m_raytracingMap->m_width);
		viewport.Height = static_cast<float>(m_raytracingMap->m_height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = m_raytracingMap->m_width;
		scissorRect.bottom = m_raytracingMap->m_height;

		GetDX12().GetCmdList()->RSSetViewports(1, &viewport);
		GetDX12().GetCmdList()->RSSetScissorRects(1, &scissorRect);


		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDX12().dsvHeap->GetCPUDescriptorHandleForHeapStart();

		GetDX12().GetCmdList()->OMSetRenderTargets(1, &m_raytracingMap->m_rtvHandles[0], FALSE, &dsvHandle);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GetDX12().GetCmdList()->ClearRenderTargetView(m_raytracingMap->m_rtvHandles[0], clearColor, 0, nullptr);
		GetDX12().GetCmdList()->ClearDepthStencilView(
			dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,
			0,
			0,
			nullptr
		);
		GetDX12().GetCmdList()->SetGraphicsRootDescriptorTable(17, m_shadowsUAV->GetGPUHandle());
		
		GetDX12().GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GetDX12().GetCmdList()->IASetVertexBuffers(0, 0, nullptr);
		GetDX12().GetCmdList()->DrawInstanced(3, 1, 0, 0);


		m_raytracingMap->TransitionState(GetDX12().GetCmdList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void RenderingManager::CalculateCompute(Scene* scene)
	{
		m_computeSkinning.Compute(scene);
	}

	void RenderingManager::UpdatePBR(Scene* scene, Camera& camera)
	{
		// Render cube maps, irradiance, prefilter and brdf maps
		RenderPbrMaps(camera);
		// Render light pass
		RenderLightPass(scene);
	}

	void RenderingManager::DebugDraw(Scene* scene, Camera& camera)
	{
		scene->GetPhysicsManager()->GetDebugDraw()->DebugDraw(GetDX12(), camera);
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