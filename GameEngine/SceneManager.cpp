#include "SceneManager.h"
#include "ErrorLogger.h"

namespace ECS
{
	SceneManager::SceneManager(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator)
		: m_device(device), m_cmdList(cmdList)
	{
		m_assetManager = std::make_shared<AssetManager>();
		m_materialManager = std::make_shared<MaterialManager>(device, cmdList, allocator);
	}



	void SceneManager::LoadScene(const std::string& sceneName)
	{
		auto scene = std::make_unique<Scene>(sceneName, m_assetManager, m_materialManager, m_device, m_cmdList);
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

	void SceneManager::Update(float dt)
	{
	}

	void SceneManager::Render(DynamicUploadBuffer* dynamicCB, Camera& camera, float dt)
	{
		GetCurrentScene()->Render(camera, dynamicCB);
	}
}
