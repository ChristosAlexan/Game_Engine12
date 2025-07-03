#include "EntityFactory.h"
#include "Scene.h"

namespace ECS
{
	EntityFactory::EntityFactory(entt::registry& registry, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
		:m_registry(&registry), m_device(device), m_cmdList(cmdList)
	{
	}

	entt::entity EntityFactory::AddEntity(Scene* scene, EntityDesc& entityDesc)
	{
		std::unique_ptr<ECS::Material> mat;

		auto id = scene->CreateEntity();
	
		auto mesh = scene->GetAssetManager()->GetOrLoadMesh(entityDesc, m_registry, id, m_device, m_cmdList);
		auto material = scene->GetMaterialManager()->GetOrCreateMaterial(entityDesc.materialDesc);
	
		TransformComponent transform{};
		m_registry->emplace<TransformComponent>(id, transform);

		RenderComponent renderComponent = {};
		renderComponent.mesh = mesh;
		renderComponent.material = material;
		renderComponent.name = entityDesc.name;
		renderComponent.hasAnimation = entityDesc.hasAnimation;

		m_registry->emplace<RenderComponent>(id, renderComponent);
		return id;
	}
}	

