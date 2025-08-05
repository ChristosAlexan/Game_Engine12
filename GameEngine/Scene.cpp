#include "Scene.h"
#include "EntityFactory.h"
#include "AnimationManager.h"
#include "MaterialManager.h"
#include "AssetManager.h"
#include "RenderingManager.h"
#include "SceneManager.h"
#include "ErrorLogger.h"



namespace ECS
{
	Scene::Scene(const std::string& sceneName, std::shared_ptr<AssetManager> assetMgr, std::shared_ptr<MaterialManager> materialMgr, 
		std::shared_ptr<RenderingManager> renderingManager,
		ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
		:m_sceneName(sceneName), m_assetManager(assetMgr),
		m_materialManager(materialMgr), m_renderingManager(renderingManager)
	{
		m_registry = entt::registry{};
		m_entityFactory = std::make_unique<EntityFactory>(m_registry, device, cmdList);
		m_lightManager = std::make_shared<LightManager>();
	}

	entt::entity Scene::CreateEntity()
	{
		return m_registry.create();
	}

	void Scene::LoadMaterials()
	{
		MaterialDesc materialDesc = {};
		materialDesc.name = "DefaultMaterial";
		materialDesc.albedoTexturePath = "Data/Textures/Tex1/plasticpattern1-albedo.png";
		materialDesc.albedoTextureName = "defaultAlbedo";
		materialDesc.normalTexturePath = "Data/Textures/Tex1/plasticpattern1-normal2b.png";
		materialDesc.normalTextureName = "defaultNormal";
		materialDesc.metalRoughnessTexturePath = "Data/Textures/Tex1/plasticpattern1-roughness2.png";
		materialDesc.metalRoughnessTextureName = "defaultMetalRougness";
		materialDesc.useAlbedoMap = true;
		materialDesc.useNormalMap = true;
		materialDesc.useMetalRoughnessMap = true;
		materialDesc.tex_format = Texture12::TEXTURE_FORMAT::AUTO;
		
		m_materialManager->GetOrCreateMaterial(materialDesc);
	}

	void Scene::LoadAssets()
	{
		std::string fpath = ".//Save files/" + GetName();
		m_saveLoadSystem.LoadScene(this, fpath);
	}

	void Scene::BuildTLAS()
	{
		GetRenderingManager()->BuildTLAS(this);
	}

	void Scene::AccumulateLights()
	{
		m_lightManager->Initialize(this);
	}

	void Scene::Update(float dt, Camera& camera, DynamicUploadBuffer* dynamicCB)
	{
		GetLightManager()->UpdateVisibleLights(GetRenderingManager()->GetDX12().GetCmdList(), camera);
		auto group = GetRegistry().group<TransformComponent, RenderComponent>();
		// Update
		for (auto [entity, transformComponent, renderComponent] : group.each())
		{
			GetAnimationManager()->Update(dt, this, entity, renderComponent);
			GetTransformManager()->Update(this, entity, transformComponent);
	
		}
	
		// Present
		for (auto [entity, transformComponent, renderComponent] : group.each())
		{
			GetRenderingManager()->RenderGbuffer(this, entity, camera, dynamicCB, transformComponent, renderComponent);
		}

		// Dispatch rays
		GetRenderingManager()->DispatchRays();
	}	

	const std::string Scene::GetName() const
	{
		return m_sceneName;
	}

	AssetManager* Scene::GetAssetManager() const
	{
		return m_assetManager.get();
	}

	MaterialManager* Scene::GetMaterialManager() const
	{
		return m_materialManager.get();
	}

	entt::registry& Scene::GetRegistry()
	{
		return m_registry;
	}

	AnimationManager* Scene::GetAnimationManager() const
	{
		return m_animationManager.get();
	}

	RenderingManager* Scene::GetRenderingManager() const
	{
		return m_renderingManager.get();
	}

	EntityFactory* Scene::GetEntityFactory() const
	{
		return m_entityFactory.get();
	}
	LightManager* Scene::GetLightManager() const
	{
		return m_lightManager.get();
	}
	TransformManager* Scene::GetTransformManager() const
	{
		return m_transformManager.get();
	}
	SaveLoadSystem& Scene::GetSaveLoadSystems()
	{
		return m_saveLoadSystem;
	}
}

