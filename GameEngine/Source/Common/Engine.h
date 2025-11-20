#pragma once

#include "AppTimer.h"
#include <future>
#include <algorithm>
#include "Camera.h"
#include "GameWindow.h"
#include <unordered_set>
#include "SceneManager.h"
#include "Metrics.h"

class Metrics;
class DX12;
class GFXGui;

class Engine
{
public:
	Engine();

	bool Initialize();
	bool StopEngine();
	void Update();

private:
	void InitializeSceneManager();
	void InitializeDirectX12();
	void CreateScenes();
private:
	AppTimer timer;
	//Camera camera;
	int width, height;
	std::unique_ptr<ECS::SceneManager> m_sceneManager;
	GameWindow game_window;

	std::unordered_set<SDL_Keycode> heldKeys;
	bool isMiddleMouseDown = false;
	bool isRightMouseDown = false;
	bool bStopEngine = false;
	int rawDeltaX = 0;
	int rawDeltaY = 0;

	Metrics m_metrics;
};
