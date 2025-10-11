#pragma once

#include "DynamicUploadBuffer.h"
#include "ConstantBufferTypes.h"
#include "Camera.h"
#include <entt/entt.hpp>
#include "SaveLoadSystem.h"
#include "RenderingECS.h"
#include "LightManager.h"
#include "TransformManager.h"
#include "EntityFactory.h"
#include "AnimationManager.h"

namespace PHYSICS
{
	class PhysicsManager;
}

namespace ECS
{
	class AssetManager;
	class MaterialManager;
	class RenderingManager;


	class Scene
	{
	public:
		Scene(const std::string& sceneName, AssetManager* assetMgr, MaterialManager* materialMgr,
			RenderingManager* renderingManager, PHYSICS::PhysicsManager* physicsManager);

		entt::entity CreateEntity();
		void LoadMaterials();
		void LoadAssets();
		void LoadPhysics();
		void AccumulateLights();
		void Update(float dt, float fps, Camera& camera);

		const std::string GetName() const;
		AssetManager* GetAssetManager() const;
		MaterialManager* GetMaterialManager() const;
		entt::registry& GetRegistry();
		AnimationManager* GetAnimationManager() const;
		RenderingManager* GetRenderingManager() const;
		PHYSICS::PhysicsManager* GetPhysicsManager() const;
		EntityFactory* GetEntityFactory() const;
		LightManager* GetLightManager() const;
		TransformManager* GetTransformManager() const;
		SaveLoadSystem& GetSaveLoadSystems();

	private:
		std::string m_sceneName;
		AssetManager* m_assetManager = nullptr;
		MaterialManager* m_materialManager = nullptr;
		RenderingManager* m_renderingManager = nullptr;
		PHYSICS::PhysicsManager* m_physicsManager = nullptr;

		std::unique_ptr<EntityFactory> m_entityFactory;
		std::unique_ptr<AnimationManager> m_animationManager;
		std::unique_ptr<LightManager> m_lightManager;
		std::unique_ptr<TransformManager> m_transformManager;
	

		UINT m_NextEntityID = 0;
		entt::registry m_registry;
		SaveLoadSystem m_saveLoadSystem;
	public:
		uint32_t blas_total = 0;
	};
}

