#include "AnimationManager.h"
#include "Scene.h"

ECS::AnimationManager::AnimationManager()
{
}

void ECS::AnimationManager::Update(float dt, Scene* scene,
	TransformComponent& transformComponent, RenderComponent& renderComponent, AnimatorComponent& animatorComponent)
{
	if (renderComponent.hasAnimation)
	{
		renderComponent.model->BuildFlatHierarchy(animatorComponent);
		renderComponent.model->CalculateFinalTransformBlend(dt, animatorComponent);
	}	
}
