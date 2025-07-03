#pragma once

namespace ECS
{
	class Scene;
	class AnimationManager
	{
	public:
		AnimationManager();
		void Update(float dt, Scene* scene);
	};

}

