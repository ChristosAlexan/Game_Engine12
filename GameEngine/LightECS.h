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
		float attenuation;
	};
}

