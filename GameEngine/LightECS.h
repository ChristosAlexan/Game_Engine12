#pragma once
#include <DirectXMath.h>

namespace ECS
{
	enum LightType
	{
		DIRECTIONAL = 0,
		SPOT = 1,
		POINT = 2
	};

	struct LightComponent
	{
		LightType lightType;
		DirectX::XMFLOAT3 color;
		float strength;
		float radius;
		float cutoff;
	};

	struct GPULight
	{
		DirectX::XMFLOAT3 position; // 12 bytes
		float strength;             // 4 bytes

		DirectX::XMFLOAT3 color;    // 12 bytes
		float radius;				// 4 bytes
		DirectX::XMFLOAT4 direction; // 16 bytes

		uint32_t lightType; // 4 bytes
		float cutoff;		// 4 bytes
		DirectX::XMFLOAT2 padding; // 8 bytes
	};
}

