#pragma once

#include "DX12.h"
#include "WindowContainer.h"
#include "AppTimer.h"
#include <future>
#include <thread>
#include <algorithm>
#include "Camera.h"

class Engine : virtual WindowContainer
{
public:
	Engine();

	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	void Update(int width, int height);

private:
	void CreateScenes(Camera& camera, int& width, int& height);
private:
	AppTimer timer;
	Camera camera;
	int width, height;
	float dt = 0.0f;
	float fps = 0.0f;
	DX12 dx12;
	std::unique_ptr<ECS::SceneManager> m_sceneManager;
	GFXGui m_gui;
};
