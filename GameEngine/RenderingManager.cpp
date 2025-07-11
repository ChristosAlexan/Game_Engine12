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

	void RenderingManager::Render(Scene* scene, Camera& camera, DynamicUploadBuffer* dynamicCB, 
		TransformComponent& transformComponent, RenderComponent& renderComponent, AnimatorComponent& animatorComponent)
	{
		if (!scene)
			return;

		m_dx12.GetCmdList()->SetPipelineState(m_dx12.pipelineState_Gbuffer.Get());

		DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&transformComponent.position);
		DirectX::XMVECTOR rot_vec = DirectX::XMLoadFloat4(&transformComponent.rotation);
		DirectX::XMVECTOR scale_vec = DirectX::XMLoadFloat3(&transformComponent.scale);

		DirectX::XMMATRIX scale_mat = DirectX::XMMatrixScalingFromVector(scale_vec);
		DirectX::XMMATRIX rot_mat = DirectX::XMMatrixRotationQuaternion(rot_vec);
		DirectX::XMMATRIX pos_mat = DirectX::XMMatrixTranslationFromVector(pos_vec);

		transformComponent.worldMatrix = scale_mat * rot_mat * pos_mat;

		CB_VS_SimpleShader vsCB = {};
		vsCB.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
		vsCB.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());
		vsCB.worldMatrix = DirectX::XMMatrixTranspose(transformComponent.worldMatrix);

		CB_VS_AnimationShader skinningCB = {};

		skinningCB.HasAnim = renderComponent.hasAnimation;
		if (renderComponent.hasAnimation && !animatorComponent.finalTransforms.empty())
		{
			memcpy(skinningCB.skinningMatrix, animatorComponent.finalTransforms.data(), sizeof(skinningCB.skinningMatrix));

		}

		CB_PS_SimpleShader psCB = {};
		psCB.lightPos = DirectX::XMFLOAT4(3.0f, 5.0f, 1.0f, 1.0f);
		psCB.color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

		if (m_dx12.GetCmdList())
		{
			if (dynamicCB)
			{
				m_dx12.GetCmdList()->SetGraphicsRootConstantBufferView(0, dynamicCB->Allocate(vsCB));
				m_dx12.GetCmdList()->SetGraphicsRootConstantBufferView(1, dynamicCB->Allocate(psCB));
				m_dx12.GetCmdList()->SetGraphicsRootConstantBufferView(3, dynamicCB->Allocate(skinningCB));
			}

			scene->GetMaterialManager()->Bindtextures(renderComponent.material.get(), m_dx12.GetCmdList(), 2);
			renderComponent.mesh->Draw(m_dx12.GetCmdList());
		}
	}
}
