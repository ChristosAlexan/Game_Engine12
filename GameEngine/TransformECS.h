#pragma once
#include "DX12Includes.h"

namespace ECS
{
	struct AABB
	{
		DirectX::XMVECTOR min = DirectX::XMVectorSet(-1.0f, -1.0f, -1.0f, 1.0f);
		DirectX::XMVECTOR max = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	};
	struct TransformComponent
	{
		DirectX::XMFLOAT3 position = { 0, 0, 0 };
		DirectX::XMFLOAT4 rotation = { 0, 0, 0, 1};
		DirectX::XMFLOAT3 scale = { 1, 1, 1 };

		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
		std::vector<DirectX::XMMATRIX> anim_transform;
		AABB aabb;
	};
}
