#pragma once
#include "MeshGenerators.h"
#include "EntityECS.h"
#include "ECSHeader.h"
#include <entt/entt.hpp>

namespace ECS
{
	class Scene;

	class EntityFactory
	{
	public:
		EntityFactory(entt::registry& registry, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
		entt::entity CreateMesh(Scene* scene, EntityDesc& entityDesc);											  
	private:
		entt::registry* m_registry = nullptr;
		ID3D12Device* m_device = nullptr;
		ID3D12GraphicsCommandList* m_cmdList = nullptr;

	};
}
