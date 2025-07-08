#include "RenderingManager.h"
#include "ConstantBufferTypes.h"
#include "Scene.h"

namespace ECS
{
	RenderingManager::RenderingManager()
	{
	}

	void RenderingManager::Render(Scene* scene, Camera& camera, DynamicUploadBuffer* dynamicCB, ID3D12GraphicsCommandList* cmdList, 
		TransformComponent& transformComponent, RenderComponent& renderComponent, AnimatorComponent& animatorComponent)
	{
		if (!scene)
			return;

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

		if (cmdList)
		{
			if (dynamicCB)
			{
				cmdList->SetGraphicsRootConstantBufferView(0, dynamicCB->Allocate(vsCB));
				cmdList->SetGraphicsRootConstantBufferView(1, dynamicCB->Allocate(psCB));
				cmdList->SetGraphicsRootConstantBufferView(3, dynamicCB->Allocate(skinningCB));
			}

			scene->GetMaterialManager()->Bindtextures(renderComponent.material.get(), cmdList, 2);
			renderComponent.mesh->Draw(cmdList);
		}
	}
}
