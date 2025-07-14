#include "RenderingManager.h"
#include "ConstantBufferTypes.h"
#include "Scene.h"
#include "ErrorLogger.h"
#include "GameWindow.h"

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
		
		InitializeRenderTargets(width, height);

		return true;
	}

	void RenderingManager::InitializeRenderTargets(int& width, int& height)
	{
		m_gBuffer.Initialize(m_dx12.GetDevice(), m_dx12.GetCmdList(), m_dx12.GetCommandAllocator(), m_dx12.GetSharedSrvHeap(), m_dx12.GetDescriptorAllocator(), width, height);
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
		m_gBuffer.GetGbufferRenderTargetTexture().Reset(m_dx12.GetCmdList());
	}

	void RenderingManager::SetGbufferRenderTarget()
	{
		m_gBuffer.GetGbufferRenderTargetTexture().SetRenderTarget(m_dx12.GetCmdList(), m_dx12.dsvHandle);
	}

	void RenderingManager::RenderGbufferFullscreen()
	{
		m_gBuffer.GetGbufferRenderTargetTexture().RenderFullScreenQuad(m_dx12.GetCmdList(), m_dx12.rtvHeap.Get(), m_dx12.dsvHeap.Get(), m_dx12.frameIndex, m_dx12.rtvDescriptorSize, m_dx12.pipelineState_2D.Get());
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

		DirectX::XMMATRIX scale_mat = DirectX::XMMatrixScalingFromVector(scale_vec);
		DirectX::XMMATRIX rot_mat = DirectX::XMMatrixRotationQuaternion(rot_vec);
		DirectX::XMMATRIX pos_mat = DirectX::XMMatrixTranslationFromVector(pos_vec);

		transformComponent.worldMatrix = scale_mat * rot_mat * pos_mat;

	
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
					memcpy(skinningCB.skinningMatrix, animatorComponent.finalTransforms.data(), sizeof(skinningCB.skinningMatrix));
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
		
		psCameraCB.cameraPos = camera.GetPositionFloat3();
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
}
