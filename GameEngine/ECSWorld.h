#pragma once
#include "ECSHeader.h"
#include "TransformECS.h"
#include "RenderingECS.h"

namespace ECS
{
	class ECSWorld
	{
	public:
		ECSWorld();

		template<typename T>
		void AddComponent(EntityID id, const T& component)
		{
			if constexpr (std::is_same_v<T, TransformComponent>)
			{
				for (const auto& [existingID, _] : m_transformComponents)
				{
					if (existingID == id)
						return; // already added
				}
				m_transformComponents.emplace(id, component);
			}
			else if constexpr (std::is_same_v<T, RenderComponent>)
			{
				for (const auto& [existingID, _] : m_renderComponents)
				{
					if (existingID == id)
						return; // already added
				}
				m_renderComponents.emplace(id, component);
			}
		}

		template<typename T>
		T* GetComponent(EntityID id)
		{
			if constexpr (std::is_same_v<T, TransformComponent>)
			{
				auto it = m_transformComponents.find(id);
				return (it != m_transformComponents.end()) ? &it->second : nullptr;
			}
			else if constexpr (std::is_same_v<T, RenderComponent>)
			{
				auto it = m_renderComponents.find(id);
				return (it != m_renderComponents.end()) ? &it->second : nullptr;
			}
		}

		template<typename T>
		void RemoveComponent(EntityID id)
		{
			if constexpr (std::is_same_v<T, TransformComponent>) 
			{
				m_transformComponents.erase(id);
			}
			else if constexpr (std::is_same_v<T, RenderComponent>) 
			{
				m_renderComponents.erase(id);
			}
		}

		const std::unordered_map<EntityID, TransformComponent>& GetAllTransformComponents() const;
		const std::unordered_map<EntityID, RenderComponent>& GetAllRenderComponents() const;
	
	private:
		std::unordered_map<EntityID, TransformComponent> m_transformComponents;
		std::unordered_map<EntityID, RenderComponent> m_renderComponents;
	};
}


