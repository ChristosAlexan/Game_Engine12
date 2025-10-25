#pragma once
#include "DX12Includes.h"
#include "Scene.h"

class GameWindow;

namespace PHYSICS
{
	class PhysicsManager;
}

namespace ECS
{
	class AssetManager;
	class MaterialManager;
	class RenderingManager;

	class SceneManager
	{
	public:
		SceneManager();
		~SceneManager();
		void InitializeManagers(GameWindow& game_window, int& width, int& height, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator);
		void AllocateRenderingManager();
		void LoadScene(const std::string& sceneName);
		void SetCurrentScene(std::string sceneName);
		Scene* GetCurrentScene() const;
		void SetupLights();
		RenderingManager* GetRenderingManager();
		void Update(float dt, float fps, Camera& camera);
		
	private:
		std::unique_ptr <AssetManager> m_assetManager;
		std::unique_ptr <MaterialManager> m_materialManager;
		std::unique_ptr <RenderingManager> m_renderingManager;
		std::unique_ptr <PHYSICS::PhysicsManager> m_physicsManager;

		std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
		Scene* m_currentScene = nullptr;
		std::string m_currentSceneName;
	};

}


