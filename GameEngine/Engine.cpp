#include "Engine.h"
#include "ErrorLogger.h"

using namespace DirectX;


Engine::Engine()
{
}

bool Engine::Initialize(std::string window_title, std::string window_class, int width, int height)
{
	this->width = width;
	this->height = height;

	timer.Start();

	if (!game_window.Initialize(width, height))
		return false;

	dx12.Initialize(game_window.GetWindow(), camera, width, height);
	if (!m_gui.Initialize(game_window.GetSDLWindow(), dx12.GetDevice(), dx12.GetCommandQueue(), dx12.GetDescriptorHeap()))
	{
		ErrorLogger::Log("Failed to initialize ImGui!");
		return false;
	}

	// Load scenes from .json files
	CreateScenes(camera, width, height);

	return true;
}

bool Engine::StopEngine()
{
	return bStopEngine;
}

void Engine::Update(int width, int height)
{
	timer.CalculateDeltaTime(dt, fps);
	timer.Restart();
	
	if (heldKeys.contains(SDLK_ESCAPE))
	{
		SDL_DestroyWindow(game_window.GetSDLWindow());
		bStopEngine = true;
	}

	// Start rendering of a frame
	dx12.StartRenderFrame(m_sceneManager.get(), m_gui, camera, width, height, dt);
	// Update current scene(animations, rendering etc.)
	m_sceneManager->Update(dt, camera, dx12.dynamicCB.get(), dx12.GetCmdList());
	m_gui.BeginRender();

	rawDeltaX = 0;
	rawDeltaY = 0;
	SDL_Event event;
	while (SDL_PollEvent(&event)) 
	{
		ImGui_ImplSDL3_ProcessEvent(&event);
		switch (event.type) {
		case SDL_EVENT_QUIT:
			SDL_DestroyWindow(game_window.GetSDLWindow());
			bStopEngine = true;
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			SDL_DestroyWindow(game_window.GetSDLWindow());
			bStopEngine = true;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event.button.button == SDL_BUTTON_MIDDLE)
				isMiddleMouseDown = true;
			else if (event.button.button == SDL_BUTTON_RIGHT)
				isRightMouseDown = true;
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event.button.button == SDL_BUTTON_MIDDLE)
				isMiddleMouseDown = false;
			else if (event.button.button == SDL_BUTTON_RIGHT)
				isRightMouseDown = false;
			break;

		case SDL_EVENT_MOUSE_MOTION:
			if (isMiddleMouseDown) 
			{
				rawDeltaX += event.motion.xrel*1.5f;
				rawDeltaY += event.motion.yrel*1.5f;
			}
			break;
		case SDL_EVENT_KEY_DOWN:
			heldKeys.insert(event.key.key);
			break;

		case SDL_EVENT_KEY_UP:
			heldKeys.erase(event.key.key);
			break;
		default:
			break;
		}
	}

	if (isMiddleMouseDown && (rawDeltaX != 0 || rawDeltaY != 0))
	{
		camera.AdjustRotation(
			static_cast<float>(rawDeltaY) * 0.004f,
			static_cast<float>(rawDeltaX) * 0.004f,
			0.0f,
			true
		);
	}
	SDL_SetWindowRelativeMouseMode(game_window.GetSDLWindow(), false);


	float cameraSpeed = 3.0f;

	if (heldKeys.contains(SDLK_LSHIFT) || heldKeys.contains(SDLK_RSHIFT))
	{
		cameraSpeed = 10.0f;
	}

	if (heldKeys.contains(SDLK_W))
	{
		camera.AdjustPosition(camera.GetForwardVector() * cameraSpeed * dt);
	}
	if (heldKeys.contains(SDLK_S))
	{
		camera.AdjustPosition(camera.GetBackwardVector() * cameraSpeed * dt);
	}
	if (heldKeys.contains(SDLK_A))
	{
		camera.AdjustPosition(camera.GetLeftVector() * cameraSpeed * dt);
	}
	if (heldKeys.contains(SDLK_D))
	{
		camera.AdjustPosition(camera.GetRightVector() * cameraSpeed * dt);
	}
	if (heldKeys.contains(SDLK_SPACE))
	{
		camera.AdjustPosition(0.0f, cameraSpeed * dt, 0.0f);
	}
	if (heldKeys.contains(SDLK_Q))
	{
		camera.AdjustPosition(0.0f, -cameraSpeed * dt, 0.0f);
	}
	if (heldKeys.contains(SDLK_F5))
	{
		std::string savePath = ".//Save files/" + m_sceneManager->GetCurrentScene()->GetName();
		m_sceneManager->GetCurrentScene()->GetSaveLoadSystems().SaveScene(m_sceneManager->GetCurrentScene(), savePath);
	}

	if (isRightMouseDown)
	{
		m_gui.SelectEntity(m_sceneManager.get(), width, height, camera);
	}
	m_gui.UpdateSelectedEntity(m_sceneManager.get(), width, height, camera);
	m_gui.UpdateAllEntities(m_sceneManager.get(), width, height, camera);

	// Finish rendering
	dx12.EndRenderFrame(m_sceneManager.get(), m_gui, camera, width, height, dt);
}

void Engine::CreateScenes(Camera& camera, int& width, int& height)
{
	dx12.ResetCommandAllocator();
	m_sceneManager = std::make_unique<ECS::SceneManager>(dx12.GetDevice(), dx12.GetCmdList(), g_descAllocator.get());
	m_sceneManager->LoadScene("Scene1", dx12.GetCmdList());
	m_sceneManager->SetCurrentScene("Scene1");
	m_sceneManager->GetCurrentScene()->LoadMaterials();
	m_sceneManager->GetCurrentScene()->LoadAssets();
	dx12.SubmitCommand();


	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	camera.PerspectiveFov(90.0f, aspectRatio, 0.1f, 100.0f);
	camera.SetPosition(0, 0, 0);
}
