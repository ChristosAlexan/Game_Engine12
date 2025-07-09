#pragma once

#include "AppTimer.h"
#include <future>
#include <algorithm>
#include "Camera.h"
#include "GameWindow.h"
#include <unordered_set>
#include "SceneManager.h"

class DX12;
class GFXGui;

class Engine
{
public:
	Engine();

	bool Initialize(std::string window_title, std::string window_class, int width, int height);
	bool StopEngine();
	void Update(int width, int height);

private:
	void InitializeSceneManager();
	void InitializeDirectX12();
	void CreateScenes(Camera& camera, int& width, int& height);
private:
	AppTimer timer;
	Camera camera;
	int width, height;
	float dt = 0.0f;
	float fps = 0.0f;
	std::unique_ptr<ECS::SceneManager> m_sceneManager;
	GameWindow game_window;

	std::unordered_set<SDL_Keycode> heldKeys;
	bool isMiddleMouseDown = false;
	bool isRightMouseDown = false;
	bool bStopEngine = false;
	int rawDeltaX = 0;
	int rawDeltaY = 0;
};
