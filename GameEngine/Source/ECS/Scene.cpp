#include "Scene.h"
#include "EntityFactory.h"
#include "AnimationManager.h"
#include "MaterialManager.h"
#include "AssetManager.h"
#include "RenderingManager.h"
#include "PhysicsManager.h"
#include "LightManager.h"
#include "TransformManager.h"
#include "EntityFactory.h"
#include "AnimationManager.h"
#include "SceneManager.h"
#include "ErrorLogger.h"
#include "MathHelpers.h"


namespace ECS
{
	Scene::Scene(const std::string& sceneName, AssetManager* assetMgr, MaterialManager* materialMgr,
		RenderingManager* renderingManager, PHYSICS::PhysicsManager* physicsManager)
		:m_sceneName(sceneName), m_assetManager(assetMgr),
		m_materialManager(materialMgr), m_renderingManager(renderingManager), m_physicsManager(physicsManager)
	{
		m_registry = entt::registry{};
		m_entityFactory = std::make_unique<EntityFactory>(m_registry, m_renderingManager->GetDX12().GetDevice(), m_renderingManager->GetDX12().GetCmdList());
		m_lightManager = std::make_unique<LightManager>();
		m_transformManager = std::make_unique<TransformManager>();
		m_animationManager = std::make_unique<AnimationManager>();

		GetCamera().SetPosition(-4.0f, 1.0f, -17.2f);
	}

	Scene::~Scene() = default;

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
	void Scene::LoadPhysics()
	{
		GetPhysicsManager()->CreatePhysicsShapes(this);
	}

	void Scene::AccumulateLights()
	{
		GetLightManager()->Initialize(this);
		GetRenderingManager()->InitializeShadowTextures(this);
	}

	void Scene::Update(float dt, float fps)
	{
		GetRenderingManager()->ResetRenderTargets();
		GetRenderingManager()->UpdateBuffers(this);

		auto group = GetRegistry().group<TransformComponent, RenderComponent>();
		for (auto [entity, transformComponent, renderComponent] : group.each())
		{
			GetAnimationManager()->Update(dt, this, entity, renderComponent);
			GetTransformManager()->Update(this, entity, transformComponent);
		}

		GetLightManager()->UpdateVisibleLights(GetRenderingManager()->GetDX12().GetCmdList(), GetCamera());
		GetPhysicsManager()->Update(this, GetCamera());

		GetRenderingManager()->CalculateCompute(this);
		GetRenderingManager()->SetGbufferRenderTarget();

		auto frustum = ExtractFrustum(DirectX::XMMatrixMultiply(GetCamera().GetViewMatrix(), GetCamera().GetProjectionMatrix()));

		for (auto [entity, transformComponent, renderComponent] : group.each())
		{
			auto& transformComponent = group.get<TransformComponent>(entity);
			auto& renderComponent = group.get<RenderComponent>(entity);

			GetRenderingManager()->RenderGbuffer(this, entity, transformComponent, renderComponent);
		}

		if (GetRenderingManager()->m_bEnableDebugDraw)
			GetRenderingManager()->DebugDraw(this);

		GetRenderingManager()->DispatchRays(this);
		GetRenderingManager()->UpdatePBR(this);
		GetPhysicsManager()->Advance(dt, fps, GetCamera());
	}

	const std::string Scene::GetName() const
	{
		return m_sceneName;
	}
	AssetManager* Scene::GetAssetManager() const
	{
		return m_assetManager;
	}
	MaterialManager* Scene::GetMaterialManager() const
	{
		return m_materialManager;
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
		return m_renderingManager;
	}
	PHYSICS::PhysicsManager* Scene::GetPhysicsManager() const
	{
		return m_physicsManager;
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
	Camera& Scene::GetCamera()
	{
		return m_camera;
	}
}

