#pragma once
#include "DX12Includes.h"
#include "MeshManager.h"
#include "MeshDataStorage.h"
#include "ECSHeader.h"
#include "MaterialECS.h"
#include "TransformECS.h"
#include "RenderingECS.h"
#include "DynamicUploadBuffer.h"
#include "Camera.h"
#include "AssetManager.h"

namespace ECS
{
	class SceneManager
	{
	public:
		SceneManager();
		void LoadAssets(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
		ECS::EntityID CreateEntity();  // Optional: overload to attach default components
		void AddTransformComponent(EntityID id, const TransformComponent& transform);
		void AddRenderComponent(EntityID id, const RenderComponent& render);
		void RenderEntities(ID3D12GraphicsCommandList* cmdList, DynamicUploadBuffer* dynamicCB, Camera& camera, float& dt);

		void UpdateTransform(EntityID id, TransformComponent& trans, float dt); // Could also include systems like transform or animation
		void Clear();          // Cleanup scene

	public:
		ECS::EntityID CreateCubeEntity(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	private:
		ECS::EntityID m_NextEntityID = 0;
		std::unordered_map<EntityID, std::string> entityNames;
		//std::unordered_map<std::string, std::shared_ptr<Mesh12>> m_meshes;
		//std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
		std::unordered_map<EntityID, TransformComponent> m_transformComponents;
		std::vector<std::pair<EntityID, RenderComponent>> m_renderComponents;
		AssetManager m_assetManager;
	};
}


