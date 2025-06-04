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

protected:
	AppTimer timer;
	Camera camera;
	int width, height;
	float dt = 0.0f;
	float fps = 0.0f;
	DX12 dx12;
};
