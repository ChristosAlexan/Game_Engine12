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
	if (!m_gui.Initialize(this->render_window.GetHWND(), dx12.GetDevice(), dx12.GetCommandQueue(), dx12.GetDescriptorHeap()))
	{
		ErrorLogger::Log("Failed to initialize ImGui!");
		return false;
	}
	CreateScenes(camera, width, height);

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
	

	dx12.StartRenderFrame(m_sceneManager.get(), m_gui, camera, width, height, dt);

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

	if (mouse.IsRightDown())
	{
		m_gui.SelectEntity(m_sceneManager.get(), width, height, camera);
	}
	m_gui.UpdateSelectedEntity(m_sceneManager.get(), width, height, camera);
	m_gui.UpdateAllEntities(m_sceneManager.get(), width, height, camera);

	dx12.EndRenderFrame(m_sceneManager.get(), m_gui, camera, width, height, dt);
}

void Engine::CreateScenes(Camera& camera, int& width, int& height)
{
	dx12.ResetCommandAllocator();
	m_sceneManager = std::make_unique<ECS::SceneManager>(dx12.GetDevice(), dx12.GetCmdList());
	m_sceneManager->LoadScene("Scene1");
	m_sceneManager->SetCurrentScene("Scene1");
	m_sceneManager->GetCurrentScene()->LoadMaterials();
	m_sceneManager->GetCurrentScene()->LoadAssets();
	dx12.SubmitCommand();


	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	camera.PerspectiveFov(90.0f, aspectRatio, 0.1f, 100.0f);
	camera.SetPosition(0, 0, 0);
}
