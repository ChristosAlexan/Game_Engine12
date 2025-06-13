#pragma once
#include "MaterialManager.h"
#include "AssetManager.h"
#include "EntityFactory.h"
#include "ECSHeader.h"
#include "TransformECS.h"
#include "RenderingECS.h"
#include "DX12_GLOBALS.h"
#include "DynamicUploadBuffer.h"
#include "ConstantBufferTypes.h"
#include "Camera.h"
#include "ECSWorld.h"

namespace ECS
{
	class Scene
	{
	public:
		Scene(const std::string& sceneName, std::shared_ptr<AssetManager> assetMgr, std::shared_ptr<MaterialManager> materialMgr,
			ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

		ECS::EntityID CreateEntity();
		void LoadMaterials();
		void LoadAssets();
		void UpdateTransform(TransformComponent& trans);

		[[nodiscard]] AssetManager* GetAssetManager() const;
		[[nodiscard]] MaterialManager* GetMaterialManager() const;
		[[nodiscard]] ECSWorld* GetWorld() const;

		void Update(float dt);
		void Render(Camera& camera, DynamicUploadBuffer* dynamicCB);

	private:
		std::string m_sceneName;
		std::shared_ptr<AssetManager> m_assetManager;
		std::shared_ptr<MaterialManager> m_materialManager;
		std::unique_ptr<EntityFactory> m_entityFactory;
		//std::unordered_map<EntityID, TransformComponent> m_transformComponents;
		//std::vector<std::pair<EntityID, RenderComponent>> m_renderComponents;

		ID3D12Device* m_device = nullptr;
		ID3D12GraphicsCommandList* m_cmdList = nullptr;

		UINT m_NextEntityID = 0;

		std::shared_ptr<ECSWorld> m_world;
	};

}

