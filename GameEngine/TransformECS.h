#pragma once
#include "DX12Includes.h"

namespace ECS
{
	struct TransformComponent
	{
		DirectX::XMFLOAT3 position = { 0, 0, 0 };
		DirectX::XMFLOAT3 rotation = { 0, 0, 0 };
		DirectX::XMFLOAT3 scale = { 1, 1, 1 };
	};
}
