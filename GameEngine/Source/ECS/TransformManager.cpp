#include "TransformManager.h"
#include "Scene.h"

namespace ECS
{
	TransformManager::TransformManager()
	{
	}

	void TransformManager::Update(Scene* scene, entt::entity& entity, ECS::TransformComponent& transformComponent)
	{
		DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&transformComponent.position);
		DirectX::XMVECTOR rot_vec = DirectX::XMLoadFloat4(&transformComponent.rotation);
		DirectX::XMVECTOR scale_vec = DirectX::XMLoadFloat3(&transformComponent.scale);

		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(scale_vec);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(rot_vec);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(pos_vec);

		transformComponent.worldMatrix = S * R * T;
	}
}

