#include "MaterialManager.h"
#include "AssetManager.h"
#include "RenderingManager.h"
#include "SceneManager.h"
#include "ErrorLogger.h"

namespace ECS
{
	SceneManager::SceneManager()
	{

	}

	void SceneManager::InitializeManagers(GameWindow& game_window, int& width, int& height, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator)
	{
		m_assetManager = std::make_shared<AssetManager>();
		m_materialManager = std::make_shared<MaterialManager>(device, cmdList, allocator);
	}
	void SceneManager::AllocateRenderingManager()
	{
		m_renderingManager = std::make_shared<ECS::RenderingManager>();
	}

	void SceneManager::LoadScene(const std::string& sceneName, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		auto scene = std::make_unique<Scene>(sceneName, m_assetManager, m_materialManager, m_renderingManager, device, cmdList);
		m_scenes.emplace(sceneName, std::move(scene));
	}

	void SceneManager::SetCurrentScene(std::string sceneName)
	{
		auto it = m_scenes.find(sceneName);
		if (it != m_scenes.end()) {
			m_currentScene = it->second.get();
			m_currentSceneName = sceneName;
		}
		else {
			ErrorLogger::Log("Scene not found: " + sceneName);
		}
	}

	Scene* SceneManager::GetCurrentScene() const
	{
		if(m_currentScene)
			return m_currentScene;

		return nullptr;
	}

	void SceneManager::SetupLights()
	{
		GetCurrentScene()->AccumulateLights();
	}

	RenderingManager* SceneManager::GetRenderingManager()
	{
		return m_renderingManager.get();
	}

	void SceneManager::Update(float dt, Camera& camera, DynamicUploadBuffer* dynamicCB)
	{
		m_currentScene->Update(dt, camera, dynamicCB);
	}
}
