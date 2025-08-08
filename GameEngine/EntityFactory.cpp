#include "EntityFactory.h"
#include "AssetManager.h"
#include "RenderingManager.h"
#include "Scene.h"
#include "MathHelpers.h"

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
	

		auto mesh = scene->GetAssetManager()->GetOrLoadMesh(scene->GetRenderingManager()->GetDX12(), entityDesc, m_registry, id, m_device, m_cmdList);
		auto material = scene->GetMaterialManager()->GetOrCreateMaterial(entityDesc.materialDesc);

		RenderComponent renderComponent = {};
		renderComponent.mesh = mesh;
		renderComponent.material = material;
		renderComponent.name = entityDesc.name;
		renderComponent.hasAnimation = entityDesc.hasAnimation;
		renderComponent.hasTextures = entityDesc.hasTextures;
		renderComponent.meshType = entityDesc.meshType;

		// Generate AABB from mesh data offline
		GenerateAABB(entityDesc.transform.aabb, &renderComponent);

		if(entityDesc.meshType == ECS::MESH_TYPE::SKELETAL_MESH || entityDesc.meshType == ECS::MESH_TYPE::STATIC_MESH)
		renderComponent.model = scene->GetAssetManager()->GetModel(entityDesc.name);

		if (entityDesc.meshType == ECS::MESH_TYPE::LIGHT)
		{
			LightComponent lightComponent;
			lightComponent.lightType = entityDesc.lightComponent.lightType;
			lightComponent.radius = entityDesc.lightComponent.radius;
			lightComponent.strength = entityDesc.lightComponent.strength;
			lightComponent.cutoff = entityDesc.lightComponent.cutoff;
			lightComponent.color = entityDesc.lightComponent.color;
			renderComponent.material->baseColor = DirectX::XMFLOAT4(lightComponent.color.x, lightComponent.color.y, lightComponent.color.z, 1.0f);

			m_registry->emplace<LightComponent>(id, lightComponent);
		}
		
		m_registry->emplace<RenderComponent>(id, renderComponent);
		m_registry->emplace<EntityDesc>(id, entityDesc);
		m_registry->emplace<TransformComponent>(id, entityDesc.transform);

		if(renderComponent.mesh->blas)
			m_registry->emplace_or_replace<BLAS*>(id, renderComponent.mesh->blas.get());

		if (entityDesc.hasAnimation)
		{
			entityDesc.animComponent = AnimatorComponent{};
			m_registry->emplace<AnimatorComponent>(id, entityDesc.animComponent);
		}
			
		
		return id;
	}
}	

