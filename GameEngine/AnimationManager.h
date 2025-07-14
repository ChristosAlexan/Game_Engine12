#pragma once
#include "RenderingECS.h"
#include <entt/entt.hpp>

namespace ECS
{
	class Scene;
	class AnimationManager
	{
	public:
		AnimationManager();
		void Update(float dt, Scene* scene, entt::entity& entity, ECS::RenderComponent& renderComponent);
	};

}

