#pragma once
#include "MaterialManager.h"
#include "AssetManager.h"
#include "EntityFactory.h"
#include "RenderingManager.h"
#include "ECSHeader.h"
//#include "TransformECS.h"
//#include "RenderingECS.h"
#include "DX12_GLOBALS.h"
#include "DynamicUploadBuffer.h"
#include "ConstantBufferTypes.h"
#include "Camera.h"
#include "AnimationManager.h"
#include <entt/entt.hpp>
#include "SaveLoadSystem.h"

namespace ECS
{
	class Scene
	{
	public:
		Scene(const std::string& sceneName, std::shared_ptr<AssetManager> assetMgr, std::shared_ptr<MaterialManager> materialMgr, 
			std::shared_ptr<RenderingManager> renderingManager,
			ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

		entt::entity CreateEntity();
		void LoadMaterials();
		void LoadAssets();
		void Update(float dt, Camera& camera, DynamicUploadBuffer* dynamicCB, ID3D12GraphicsCommandList* cmdList);

		AABB GetWorldAABB(TransformComponent* trans, RenderComponent* renderComp);
		const std::string GetName() const;
		AssetManager* GetAssetManager() const;
		MaterialManager* GetMaterialManager() const;
		entt::registry& GetRegistry();
		AnimationManager* GetAnimationManager() const;
		RenderingManager* GetRenderingManager() const;
		EntityFactory* GetEntityFactory() const;
		SaveLoadSystem& GetSaveLoadSystems();

	private:
		AABB GenerateAABB(AABB& aabb, DirectX::XMMATRIX& worldMatrix, RenderComponent* renderComp);
	private:
		std::string m_sceneName;
		std::shared_ptr<AssetManager> m_assetManager;
		std::shared_ptr<MaterialManager> m_materialManager;
		std::shared_ptr<EntityFactory> m_entityFactory;
		std::shared_ptr<AnimationManager> m_animationManager;
		std::shared_ptr<RenderingManager> m_renderingManager;

		UINT m_NextEntityID = 0;
		entt::registry m_registry;
		SaveLoadSystem m_saveLoadSystem;
	};
}

