#pragma once
#include "DX12Includes.h"
#include "Scene.h"

namespace ECS
{
	class SceneManager
	{
	public:
		SceneManager(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator);
		void InitDescAllocator(ID3D12DescriptorHeap* heap);
		void LoadScene(const std::string& sceneName, ID3D12GraphicsCommandList* cmdList);
		void SetCurrentScene(std::string sceneName);
		Scene* GetCurrentScene() const;
		void Update(float dt, Camera& camera, DynamicUploadBuffer* dynamicCB, ID3D12GraphicsCommandList* cmdList);
		
	private:
		ID3D12Device* m_device = nullptr;
		std::shared_ptr <AssetManager> m_assetManager;
		std::shared_ptr <MaterialManager> m_materialManager;
		std::shared_ptr <RenderingManager> m_renderingManager;

		std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
		Scene* m_currentScene = nullptr;
		std::string m_currentSceneName;
	};

}


