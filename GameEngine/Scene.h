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
#include "AnimationManager.h"
#include <entt/entt.hpp>

namespace ECS
{
	class Scene
	{
	public:
		Scene(const std::string& sceneName, std::shared_ptr<AssetManager> assetMgr, std::shared_ptr<MaterialManager> materialMgr,
			ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

		entt::entity CreateEntity();
		void LoadMaterials();
		void LoadAssets();
		void Update(float dt);

		AABB GetWorldAABB(TransformComponent* trans, RenderComponent* renderComp);
		[[nodiscard]] const std::string GetName() const;
		[[nodiscard]] AssetManager* GetAssetManager() const;
		[[nodiscard]] MaterialManager* GetMaterialManager() const;
		[[nodiscard]] entt::registry& GetRegistry();
		[[nodiscard]] AnimationManager* GetAnimationManager() const;

		void Render(Camera& camera, DynamicUploadBuffer* dynamicCB);
	private:
		AABB GenerateAABB(AABB& aabb, DirectX::XMMATRIX& worldMatrix, RenderComponent* renderComp);
	private:
		std::string m_sceneName;
		std::shared_ptr<AssetManager> m_assetManager;
		std::shared_ptr<MaterialManager> m_materialManager;
		std::unique_ptr<EntityFactory> m_entityFactory;
		std::unique_ptr<AnimationManager> m_animationManager;

		ID3D12Device* m_device = nullptr;
		ID3D12GraphicsCommandList* m_cmdList = nullptr;

		UINT m_NextEntityID = 0;

		entt::registry m_registry;
	};

}

