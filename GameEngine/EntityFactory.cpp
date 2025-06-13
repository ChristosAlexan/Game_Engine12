#include "EntityFactory.h"
#include "Scene.h"

namespace ECS
{
	EntityFactory::EntityFactory(ECSWorld* world, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
		:m_world(world), m_device(device), m_cmdList(cmdList)
	{
	}

	EntityID EntityFactory::CreateStaticMesh(Scene* scene, EntityDesc& entityDesc)
	{
		EntityID id = scene->CreateEntity();

		auto mesh = scene->GetAssetManager()->GetOrLoadMesh(entityDesc.meshType, entityDesc.name, m_device, m_cmdList);
		auto material = scene->GetMaterialManager()->GetOrCreateMaterial(entityDesc.materialDesc, m_device, m_cmdList, g_descAllocator.get());

		TransformComponent transform{};
		AddComponent(id, transform);

		RenderComponent renderComponent = {};
		renderComponent.mesh = mesh;
		renderComponent.material = material;
		AddComponent(id, renderComponent);

		return id;
	}
}	

