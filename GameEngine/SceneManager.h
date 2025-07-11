#pragma once
#include "DX12Includes.h"
#include "Scene.h"

class GameWindow;
namespace ECS
{
	class AssetManager;
	class MaterialManager;
	class RenderingManager;

	class SceneManager
	{
	public:
		SceneManager();
		void InitializeManagers(GameWindow& game_window, int& width, int& height, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator);
		void AllocateRenderingManager();
		void LoadScene(const std::string& sceneName, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
		void SetCurrentScene(std::string sceneName);
		Scene* GetCurrentScene() const;
		RenderingManager* GetRenderingManager();
		void Update(float dt, Camera& camera, DynamicUploadBuffer* dynamicCB);
		
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


