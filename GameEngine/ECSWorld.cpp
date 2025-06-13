#include "ECSWorld.h"

namespace ECS
{
	ECSWorld::ECSWorld()
	{
	}

	const std::unordered_map<EntityID, TransformComponent>& ECSWorld::GetAllTransformComponents() const
	{
		return m_transformComponents;
	}

	const std::unordered_map<EntityID, RenderComponent>& ECSWorld::GetAllRenderComponents() const
	{
		return m_renderComponents;
	}

}
