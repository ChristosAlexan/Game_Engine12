#include "Engine.h"

using namespace DirectX;


Engine::Engine()
{
}

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	this->width = width;
	this->height = height;

	timer.Start();

	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
		return false;


	dx12.Initialize(this->render_window.GetHWND(), camera, width, height);

	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update(int width, int height)
{
	timer.CalculateDeltaTime(dt, fps);
	timer.Restart();
	
	if (keyboard.KeyIsPressed(VK_ESCAPE))
	{
		this->render_window.~RenderWindow();
	}

	float cameraSpeed = 3.0f;


	while (!mouse.EventBufferIsEmpty())
	{
		MouseEvent me = mouse.ReadEvent();
	
		if (mouse.IsMiddleDown())
		{
			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				camera.AdjustRotation(static_cast<float>(me.GetPosY()) * 0.004f, static_cast<float>(me.GetPosX()) * 0.004f, 0.0f, true);
			}
		}
	}

	if (keyboard.KeyIsPressed(VK_SHIFT))
	{
		cameraSpeed = 10.0f;
	}
	if (keyboard.KeyIsPressed('W'))
	{
		camera.AdjustPosition(camera.GetForwardVector() * cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed('S'))
	{
		camera.AdjustPosition(camera.GetBackwardVector() * cameraSpeed * dt);
	}

	if (keyboard.KeyIsPressed('A'))
	{
		camera.AdjustPosition(camera.GetLeftVector() * cameraSpeed * dt);

	}
	if (keyboard.KeyIsPressed('D'))
	{
		camera.AdjustPosition(camera.GetRightVector() * cameraSpeed * dt);
	}

	if (keyboard.KeyIsPressed(VK_SPACE))
	{
		camera.AdjustPosition(0.0f, cameraSpeed * dt, 0.0f);
	}
	if (keyboard.KeyIsPressed('Q'))
	{
		camera.AdjustPosition(0.0f, -cameraSpeed * dt, 0.0f);
	}



	ClipCursor(NULL);
	while (ShowCursor(TRUE) < 0);
	dx12.RenderFrame(camera, width, height, dt);
}
