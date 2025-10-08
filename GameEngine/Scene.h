#pragma once

#include "DynamicUploadBuffer.h"
#include "ConstantBufferTypes.h"
#include "Camera.h"
#include <entt/entt.hpp>
#include "SaveLoadSystem.h"
#include "RenderingECS.h"
#include "LightManager.h"
#include "TransformManager.h"

namespace PHYSICS
{
	class PhysicsManager;
}

namespace ECS
{
	class EntityFactory;
	class AnimationManager;
	class AssetManager;
	class MaterialManager;
	class RenderingManager;


	class Scene
	{
	public:
		Scene(const std::string& sceneName, std::shared_ptr<AssetManager> assetMgr, std::shared_ptr<MaterialManager> materialMgr, 
			std::shared_ptr<RenderingManager> renderingManager, std::shared_ptr<PHYSICS::PhysicsManager> physicsManager);

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
		std::shared_ptr<AssetManager> m_assetManager;
		std::shared_ptr<MaterialManager> m_materialManager;
		std::shared_ptr<EntityFactory> m_entityFactory;
		std::shared_ptr<AnimationManager> m_animationManager;
		std::shared_ptr<RenderingManager> m_renderingManager;
		std::shared_ptr<LightManager> m_lightManager;
		std::shared_ptr<TransformManager> m_transformManager;
		std::shared_ptr<PHYSICS::PhysicsManager> m_physicsManager;

		UINT m_NextEntityID = 0;
		entt::registry m_registry;
		SaveLoadSystem m_saveLoadSystem;
	public:
		uint32_t blas_total = 0;
	};
}

