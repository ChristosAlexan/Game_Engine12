#include "AnimationManager.h"
#include "Scene.h"
#include "ModelData.h"

ECS::AnimationManager::AnimationManager()
{
}

void ECS::AnimationManager::Update(float dt, Scene* scene, entt::entity& entity, ECS::RenderComponent& renderComponent)
{
	if (renderComponent.hasAnimation)
	{
		if (scene->GetRegistry().all_of<AnimatorComponent>(entity))
		{
			AnimatorComponent& animatorComponent = scene->GetRegistry().get<AnimatorComponent>(entity);
			renderComponent.model->BuildFlatHierarchy(animatorComponent);
			renderComponent.model->CalculateFinalTransformBlend(dt, animatorComponent);
		}
		
	}	
}
