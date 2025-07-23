#include "RenderingManager.h"
#include "ConstantBufferTypes.h"
#include "Scene.h"
#include "ErrorLogger.h"
#include "GameWindow.h"
#include <cassert>

namespace ECS
{
	RenderingManager::RenderingManager()
	{
	}

	bool RenderingManager::Initialize(GameWindow& game_window, int width, int height)
	{
	
		m_dx12.Initialize(game_window.GetWindow(), width, height);
		if (!m_gui.Initialize(game_window.GetSDLWindow(), m_dx12.GetDevice(), m_dx12.GetCommandQueue(), m_dx12.GetRtvHeap(), m_dx12.GetDescriptorAllocator()))
		{
			ErrorLogger::Log("Failed to initialize ImGui!");
			return false;
		}
		
		return true;
	}

	void RenderingManager::InitializeRenderTargets(int& width, int& height)
	{
		hdr_map1.Initialize(m_dx12.GetDevice(), m_dx12.GetCmdList(), m_dx12.GetDescriptorAllocator(), "Data/HDR/warm_restaurant_night_2k.hdr");

		m_gBuffer.Initialize(m_dx12.GetDevice(), m_dx12.GetCmdList(), m_dx12.GetCommandAllocator(), m_dx12.GetSharedSrvHeap(), m_dx12.GetDescriptorAllocator(), width, height);
		m_cubeMap1.Initialize(m_dx12.GetDevice(), m_dx12.GetCmdList(), m_dx12.GetCommandAllocator(), m_dx12.GetSharedSrvHeap(), m_dx12.GetDescriptorAllocator(), 512, 512);
		m_irradianceMap.Initialize(m_dx12.GetDevice(), m_dx12.GetCmdList(), m_dx12.GetCommandAllocator(), m_dx12.GetSharedSrvHeap(), m_dx12.GetDescriptorAllocator(), 64, 64);
		m_prefilterMap.Initialize(m_dx12.GetDevice(), m_dx12.GetCmdList(), m_dx12.GetCommandAllocator(), m_dx12.GetSharedSrvHeap(), m_dx12.GetDescriptorAllocator(), 512, 512, 5);

		std::vector<DXGI_FORMAT> formats = {DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT};
		m_brdfMap.Initialize(m_dx12.GetDevice(), m_dx12.GetCmdList(), m_dx12.GetCommandAllocator(), m_dx12.GetSharedSrvHeap(), m_dx12.GetDescriptorAllocator(), 512, 512, formats, 1);
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
		m_cubeMap1.GetCubeMapRenderTargetTexture().Reset(m_dx12.GetCmdList());
		m_irradianceMap.GetCubeMapRenderTargetTexture().Reset(m_dx12.GetCmdList());
		m_prefilterMap.GetCubeMapRenderTargetTexture().Reset(m_dx12.GetCmdList());
		m_gBuffer.GetGbufferRenderTargetTexture().Reset(m_dx12.GetCmdList());
		m_brdfMap.Reset(m_dx12.GetCmdList());
	}

	void RenderingManager::SetRenderTarget(RenderTargetTexture& renderTarget, float* clearColor)
	{
		renderTarget.SetRenderTarget(m_dx12.GetCmdList(), m_dx12.dsvHandle, clearColor);
	}

	void RenderingManager::LightPass(Scene* scene)
	{
		RenderLightPass(scene, m_gBuffer.GetGbufferRenderTargetTexture(), 4);
	}

	void RenderingManager::RenderPbrPass(Camera& camera, DynamicUploadBuffer* dynamicCB)
	{
		if (bRenderPbrPass)
		{
			m_cubeMap1.Render(GetDX12(), camera, m_dx12.pipelineState_Cubemap.Get(), 8, hdr_map1.GetHDRtexture().GetGPUHandle());
			// Render the irradiance map
			m_irradianceMap.Render(GetDX12(), camera, m_dx12.pipelineState_IrradianceConv.Get(), 9, m_cubeMap1.GetCubeMapRenderTargetTexture().GetSrvGpuHandle(0));
			// Render the prefilter map
			m_prefilterMap.RenderMips(GetDX12(), camera, m_dx12.pipelineState_Prefilter.Get(), 9, m_cubeMap1.GetCubeMapRenderTargetTexture().GetSrvGpuHandle(0));
			// Render the brdf map
			RenderBRDF(m_brdfMap);
			bRenderPbrPass = false;
		}
	}

	void RenderingManager::Render(Scene* scene, entt::entity& entity, Camera& camera, DynamicUploadBuffer* dynamicCB, 
		TransformComponent& transformComponent, RenderComponent& renderComponent)
	{
		if (!scene)
			return;

		CB_VS_SimpleShader vsCB = {};
		CB_VS_AnimationShader skinningCB = {};
		CB_PS_SimpleShader psCB = {};
		CB_PS_Material psMaterialCB = {};
		CB_PS_Camera psCameraCB = {};

		m_dx12.GetCmdList()->SetPipelineState(m_dx12.pipelineState_Gbuffer.Get());

		DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&transformComponent.position);
		DirectX::XMVECTOR rot_vec = DirectX::XMLoadFloat4(&transformComponent.rotation);
		DirectX::XMVECTOR scale_vec = DirectX::XMLoadFloat3(&transformComponent.scale);

		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(scale_vec);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(rot_vec);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(pos_vec);

		transformComponent.worldMatrix = S * R * T;

	
		vsCB.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
		vsCB.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());
		vsCB.worldMatrix = DirectX::XMMatrixTranspose(transformComponent.worldMatrix);


		skinningCB.HasAnim = renderComponent.hasAnimation;

		if (renderComponent.hasAnimation)
		{
			if (scene->GetRegistry().all_of<AnimatorComponent>(entity))
			{
				AnimatorComponent& animatorComponent = scene->GetRegistry().get<AnimatorComponent>(entity);
				if (!animatorComponent.finalTransforms.empty())
				{
					size_t matrixCount = animatorComponent.finalTransforms.size();
					assert(matrixCount <= sizeof(skinningCB.skinningMatrix));
					memcpy(skinningCB.skinningMatrix, animatorComponent.finalTransforms.data(), matrixCount * sizeof(DirectX::XMMATRIX));
				
				}
			}
		}
		
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


		if (m_dx12.GetCmdList())
		{
			if (dynamicCB)
			{
				m_dx12.GetCmdList()->SetGraphicsRootConstantBufferView(0, dynamicCB->Allocate(vsCB));
				m_dx12.GetCmdList()->SetGraphicsRootConstantBufferView(1, dynamicCB->Allocate(psCB));
				m_dx12.GetCmdList()->SetGraphicsRootConstantBufferView(3, dynamicCB->Allocate(skinningCB));
				m_dx12.GetCmdList()->SetGraphicsRootConstantBufferView(5, dynamicCB->Allocate(psMaterialCB));
				m_dx12.GetCmdList()->SetGraphicsRootConstantBufferView(6, dynamicCB->Allocate(psCameraCB));
			}

			if(renderComponent.hasTextures)
				scene->GetMaterialManager()->Bindtextures(renderComponent.material.get(), m_dx12.GetCmdList(), 2);
			renderComponent.mesh->Draw(m_dx12.GetCmdList());
		}
	}

	void RenderingManager::RenderBRDF(RenderTargetTexture& renderTexture)
	{
		ID3D12DescriptorHeap* heaps[] = { m_dx12.GetSharedSrvHeap() };
		m_dx12.GetCmdList()->SetDescriptorHeaps(1, heaps);
		m_dx12.GetCmdList()->SetPipelineState(m_dx12.pipelineState_Brdf.Get());


		float aspect = static_cast<float>(renderTexture.m_width) / static_cast<float>(renderTexture.m_height);
		float nearZ = 0.01f;
		float farZ = 1000.0f;
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspect, nearZ, farZ);
		DirectX::XMVECTOR position = DirectX::XMVectorZero();
		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(renderTexture.m_width);
		viewport.Height = static_cast<float>(renderTexture.m_height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = renderTexture.m_width;
		scissorRect.bottom = renderTexture.m_height;

		m_dx12.GetCmdList()->RSSetViewports(1, &viewport);
		m_dx12.GetCmdList()->RSSetScissorRects(1, &scissorRect);


		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dx12.dsvHeap->GetCPUDescriptorHandleForHeapStart();
		
		m_dx12.GetCmdList()->OMSetRenderTargets(1, &renderTexture.m_rtvHandles[0], FALSE, &dsvHandle);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_dx12.GetCmdList()->ClearRenderTargetView(renderTexture.m_rtvHandles[0], clearColor, 0, nullptr);
		m_dx12.GetCmdList()->ClearDepthStencilView(
			dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,
			0,
			0,
			nullptr
		);
	
		m_dx12.GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_dx12.GetCmdList()->IASetVertexBuffers(0, 0, nullptr);
		m_dx12.GetCmdList()->DrawInstanced(3, 1, 0, 0);

		// Transition to SRV
		renderTexture.TransitionToSRV(m_dx12.GetCmdList());
	}

	void RenderingManager::RenderLightPass(Scene* scene, RenderTargetTexture& renderTexture, UINT rootParameterIndex)
	{
		CB_PS_LIGHTS lights_data = {};

		ID3D12DescriptorHeap* heaps[] = { m_dx12.GetSharedSrvHeap()};
		m_dx12.GetCmdList()->SetDescriptorHeaps(1, heaps);
		// Transition to SRV
		renderTexture.TransitionToSRV(m_dx12.GetCmdList());
		// Set render target
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_dx12.GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), m_dx12.frameIndex, m_dx12.rtvDescriptorSize);
		// Set depth-stencil
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dx12.dsvHeap->GetCPUDescriptorHandleForHeapStart();
		m_dx12.GetCmdList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		m_dx12.GetCmdList()->SetGraphicsRootDescriptorTable(rootParameterIndex, renderTexture.GetSrvGpuHandle(0));
		m_dx12.GetCmdList()->SetGraphicsRootDescriptorTable(11, m_prefilterMap.GetCubeMapRenderTargetTexture().GetSrvGpuHandle(0));
		m_dx12.GetCmdList()->SetGraphicsRootDescriptorTable(12, m_irradianceMap.GetCubeMapRenderTargetTexture().GetSrvGpuHandle(0));
		m_dx12.GetCmdList()->SetGraphicsRootDescriptorTable(13, m_brdfMap.GetSrvGpuHandle(0));

		// Get all the light components in the scene
		auto view = scene->GetRegistry().view<LightComponent>();
		std::size_t totalLights = view.size();
		lights_data.totalLights = totalLights;
		lights_data.padding3 = DirectX::XMFLOAT3(0, 0, 0);

		if (m_dx12.dynamicCB)
		{
			m_dx12.GetCmdList()->SetGraphicsRootConstantBufferView(14, m_dx12.dynamicCB->Allocate(lights_data));
		}
	
		m_dx12.GetCmdList()->SetPipelineState(m_dx12.pipelineState_2D.Get());
		m_dx12.GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_dx12.GetCmdList()->IASetVertexBuffers(0, 0, nullptr);
		m_dx12.GetCmdList()->DrawInstanced(3, 1, 0, 0);
	}
}