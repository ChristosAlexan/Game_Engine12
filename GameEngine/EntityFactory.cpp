#include "EntityFactory.h"
#include "Scene.h"

namespace ECS
{
	EntityFactory::EntityFactory(entt::registry& registry, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
		:m_registry(&registry), m_device(device), m_cmdList(cmdList)
	{
	}

	entt::entity EntityFactory::CreateMesh(Scene* scene, EntityDesc& entityDesc)
	{
		auto id = scene->CreateEntity();

		auto mesh = scene->GetAssetManager()->GetOrLoadMesh(entityDesc.meshType, entityDesc.name, m_device, m_cmdList, &entityDesc.filePath);
		auto material = scene->GetMaterialManager()->GetOrCreateMaterial(entityDesc.materialDesc);

		TransformComponent transform{};
		m_registry->emplace<TransformComponent>(id, transform);

		RenderComponent renderComponent = {};
		renderComponent.mesh = mesh;
		renderComponent.material = material;
		m_registry->emplace<RenderComponent>(id, renderComponent);
		return id;
	}
}	

