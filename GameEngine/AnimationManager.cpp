#include "AnimationManager.h"
#include "Scene.h"

ECS::AnimationManager::AnimationManager()
{
}

void ECS::AnimationManager::Update(float dt, Scene* scene)
{
	auto animView = scene->GetRegistry().view<Model>();
	auto transformRenderGroup = scene->GetRegistry().group<TransformComponent, RenderComponent>();

	for (auto [entity, model] : animView.each())
	{
		for (auto [entity2, transform, renderComponent] : transformRenderGroup.each())
		{
			if (renderComponent.name == model.name && renderComponent.hasAnimation)
			{

				model.CalculateFinalTransform(dt, transform.anim_transform);

				//OutputDebugStringA(("Anim Size = " + std::to_string(transform.anim_transform.size()) + "\n").c_str());
			}	
		}
	}
}
