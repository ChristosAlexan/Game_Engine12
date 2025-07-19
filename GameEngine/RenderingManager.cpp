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
		m_gBuffer.Initialize(m_dx12.GetDevice(), m_dx12.GetCmdList(), m_dx12.GetCommandAllocator(), m_dx12.GetSharedSrvHeap(), m_dx12.GetDescriptorAllocator(), width, height);
		m_cubeMap1.Initialize(m_dx12.GetDevice(), m_dx12.GetCmdList(), m_dx12.GetCommandAllocator(), m_dx12.GetSharedSrvHeap(), m_dx12.GetDescriptorAllocator());
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
		m_gBuffer.GetGbufferRenderTargetTexture().Reset(m_dx12.GetCmdList());
	}

	void RenderingManager::SetRenderTarget(RenderTargetTexture& renderTarget, float* clearColor)
	{
		renderTarget.SetRenderTarget(m_dx12.GetCmdList(), m_dx12.dsvHandle, clearColor);
	}

	void RenderingManager::RenderGbufferFullscreen()
	{
		RenderFullScreenQuad(m_gBuffer.GetGbufferRenderTargetTexture(), 4);
	}

	void RenderingManager::RenderCubeMap(Camera& camera, DynamicUploadBuffer* dynamicCB, CubeMap& cubeMap)
	{
		if (cubeMap.bRender)
		{
			cubeMap.Render(GetDX12(), camera, m_dx12.pipelineState_Cubemap.Get(), 8);
			cubeMap.bRender = false;
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

	void RenderingManager::RenderFullScreenQuad(RenderTargetTexture& renderTexture, UINT rootParameterIndex)
	{
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
		m_dx12.GetCmdList()->SetGraphicsRootDescriptorTable(9, m_cubeMap1.GetCubeMapRenderTargetTexture().GetSrvGpuHandle(0));
		//m_dx12.GetCmdList()->SetGraphicsRootDescriptorTable(8, m_cubeMap1.hdr_map1.GetHDRtexture().GetGPUHandle());
		m_dx12.GetCmdList()->SetPipelineState(m_dx12.pipelineState_2D.Get());
		m_dx12.GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_dx12.GetCmdList()->IASetVertexBuffers(0, 0, nullptr);
		m_dx12.GetCmdList()->DrawInstanced(3, 1, 0, 0);
	}
}