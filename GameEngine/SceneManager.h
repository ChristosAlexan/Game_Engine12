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
		void LoadScene(const std::string& sceneName);
		void SetCurrentScene(std::string sceneName);
		[[nodiscard]] Scene* GetCurrentScene() const;
		void Update(float dt);
		void Render(DynamicUploadBuffer* dynamicCB, Camera& camera, float dt);
		

	private:
		ID3D12Device* m_device = nullptr;
		ID3D12GraphicsCommandList* m_cmdList;

		std::shared_ptr <AssetManager> m_assetManager;
		std::shared_ptr <MaterialManager> m_materialManager;

		std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
		Scene* m_currentScene = nullptr;
		std::string m_currentSceneName;
	};

}


