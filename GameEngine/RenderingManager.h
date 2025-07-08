#pragma once
#include "Camera.h"
#include "DynamicUploadBuffer.h"
#include "RenderingECS.h"
#include "ModelData.h"
#include "TransformECS.h"

namespace ECS
{
	class Scene;
	class RenderingManager
	{
	public:
		RenderingManager();
		void Render(Scene* scene, Camera& camera, DynamicUploadBuffer* dynamicCB, ID3D12GraphicsCommandList* cmdList,
			TransformComponent& transformComponent, RenderComponent& renderComponent, AnimatorComponent& animatorComponent);
	};
}


