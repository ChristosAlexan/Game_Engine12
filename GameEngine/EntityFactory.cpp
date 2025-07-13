#include "EntityFactory.h"
#include "AssetManager.h"
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
	
		entityDesc.animComponent = AnimatorComponent{};

		auto mesh = scene->GetAssetManager()->GetOrLoadMesh(entityDesc, m_registry, id, m_device, m_cmdList);
		auto material = scene->GetMaterialManager()->GetOrCreateMaterial(entityDesc.materialDesc);

		RenderComponent renderComponent = {};
		renderComponent.mesh = mesh;
		renderComponent.material = material;
		renderComponent.name = entityDesc.name;
		renderComponent.hasAnimation = entityDesc.hasAnimation;
		renderComponent.hasTextures = entityDesc.hasTextures;
		renderComponent.meshType = entityDesc.meshType;

		if(entityDesc.meshType == ECS::MESH_TYPE::SKELETAL_MESH || entityDesc.meshType == ECS::MESH_TYPE::STATIC_MESH)
		renderComponent.model = scene->GetAssetManager()->GetModel(entityDesc.name);

		if (entityDesc.meshType == ECS::MESH_TYPE::LIGHT)
		{
			LightComponent lightComponent;
			lightComponent.attenuation = 1.0f;
			lightComponent.color = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
			renderComponent.material->baseColor = DirectX::XMFLOAT4(lightComponent.color.x, lightComponent.color.y, lightComponent.color.z, 1.0f);

			m_registry->emplace<LightComponent>(id, lightComponent);
		}
		m_registry->emplace<RenderComponent>(id, renderComponent);
		m_registry->emplace<EntityDesc>(id, entityDesc);
		m_registry->emplace<TransformComponent>(id, entityDesc.transform);
		m_registry->emplace<AnimatorComponent>(id, entityDesc.animComponent);
		
		return id;
	}
}	

