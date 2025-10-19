#pragma once
#include "TransformECS.h"
#include <entt/entt.hpp>

namespace ECS
{
	class Scene;
	class TransformManager
	{
	public:
		TransformManager();
		void Update(Scene* scene, entt::entity& entity, ECS::TransformComponent& transformComponent);
	};
}


