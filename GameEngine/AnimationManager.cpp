#include "AnimationManager.h"
#include "Scene.h"

ECS::AnimationManager::AnimationManager()
{
}

void ECS::AnimationManager::Update(float dt, Scene* scene)
{
	auto animView = scene->GetRegistry().view<Model>();
	auto transformRenderGroup = scene->GetRegistry().group<TransformComponent, RenderComponent, AnimatorComponent>();

	for (auto [entity, model] : animView.each())
	{
		for (auto [entity2, transform, renderComponent, AnimComponent] : transformRenderGroup.each())
		{
			if (renderComponent.name == model.name && renderComponent.hasAnimation)
			{
				model.BuildFlatHierarchy(AnimComponent);
				model.CalculateFinalTransformBlend(dt, AnimComponent);
			}	
		}
	}
}
