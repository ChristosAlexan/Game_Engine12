#include "Engine.h"
#include "GFXGui.h"
#include "RenderingManager.h"
#include "ErrorLogger.h"
#include "Metrics.h"

using namespace DirectX;


Engine::Engine()
{
}

bool Engine::Initialize()
{
	timer.Start();

	if (!game_window.Initialize())
		return false;

	this->width = game_window.GetScreenWidth();
	this->height = game_window.GetScreenHeight();

	InitializeSceneManager();
	InitializeDirectX12();
	// Load scenes from .json files
	CreateScenes();

	return true;
}

bool Engine::StopEngine()
{
	return bStopEngine;
}

void Engine::Update()
{
	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	D3D12_RECT scissorRect = { 0, 0, width, height };

	timer.CalculateDeltaTime(m_metrics.dt, m_metrics.fps);
	timer.GetAverageFPS(m_metrics);
	m_metrics.ms = timer.GetMilliseconds();
	timer.Restart();

	if (heldKeys.contains(SDLK_ESCAPE))
	{
		bStopEngine = true;
	}

	// Start rendering of a frame
	m_sceneManager->GetRenderingManager()->GetDX12().StartRenderFrame(m_sceneManager->GetRenderingManager()->GetGFXGui(), m_sceneManager->GetCurrentScene()->GetCamera(), width, height, m_metrics.dt);
	// Update current scene(animations, rendering etc.)
	m_sceneManager->Update(m_metrics.dt, m_metrics.fps);
	m_sceneManager->GetRenderingManager()->GetGFXGui().BeginRender();

	rawDeltaX = 0;
	rawDeltaY = 0;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL3_ProcessEvent(&event);
		switch (event.type) {
		case SDL_EVENT_QUIT:
			bStopEngine = true;
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
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
				rawDeltaX += event.motion.xrel * 1.5f;
				rawDeltaY += event.motion.yrel * 1.5f;
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
		m_sceneManager->GetCurrentScene()->GetCamera().AdjustRotation(
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
		m_sceneManager->GetCurrentScene()->GetCamera().AdjustPosition(m_sceneManager->GetCurrentScene()->GetCamera().GetForwardVector() * cameraSpeed * m_metrics.dt);
	}
	if (heldKeys.contains(SDLK_S))
	{
		m_sceneManager->GetCurrentScene()->GetCamera().AdjustPosition(m_sceneManager->GetCurrentScene()->GetCamera().GetBackwardVector() * cameraSpeed * m_metrics.dt);
	}
	if (heldKeys.contains(SDLK_A))
	{
		m_sceneManager->GetCurrentScene()->GetCamera().AdjustPosition(m_sceneManager->GetCurrentScene()->GetCamera().GetLeftVector() * cameraSpeed * m_metrics.dt);
	}
	if (heldKeys.contains(SDLK_D))
	{
		m_sceneManager->GetCurrentScene()->GetCamera().AdjustPosition(m_sceneManager->GetCurrentScene()->GetCamera().GetRightVector() * cameraSpeed * m_metrics.dt);
	}
	if (heldKeys.contains(SDLK_SPACE))
	{
		m_sceneManager->GetCurrentScene()->GetCamera().AdjustPosition(0.0f, cameraSpeed * m_metrics.dt, 0.0f);
	}
	if (heldKeys.contains(SDLK_Q))
	{
		m_sceneManager->GetCurrentScene()->GetCamera().AdjustPosition(0.0f, -cameraSpeed * m_metrics.dt, 0.0f);
	}
	if (heldKeys.contains(SDLK_F5))
	{
		std::string savePath = ".//Save files/" + m_sceneManager->GetCurrentScene()->GetName();
		m_sceneManager->GetCurrentScene()->GetSaveLoadSystems().SaveScene(m_sceneManager->GetCurrentScene(), savePath);
	}

	if (isRightMouseDown)
	{
		m_sceneManager->GetRenderingManager()->GetGFXGui().SelectEntity(m_sceneManager.get(), width, height, m_sceneManager->GetCurrentScene()->GetCamera());
	}

	m_sceneManager->GetRenderingManager()->GetGFXGui().GeneralGuiSettings(m_sceneManager.get(), m_metrics);
	m_sceneManager->GetRenderingManager()->GetGFXGui().UpdateSelectedEntity(m_sceneManager.get(), width, height, m_sceneManager->GetCurrentScene()->GetCamera());
	m_sceneManager->GetRenderingManager()->GetGFXGui().SelectEntityList(m_sceneManager.get(), width, height, m_sceneManager->GetCurrentScene()->GetCamera());

	m_sceneManager->GetRenderingManager()->GetDX12().EndRenderFrame(m_sceneManager->GetRenderingManager()->GetGFXGui(), m_sceneManager->GetCurrentScene()->GetCamera(), width, height, m_metrics.dt);

	if (bStopEngine)
	{
		SDL_DestroyWindow(game_window.GetSDLWindow());
		return;
	}
}

void Engine::InitializeSceneManager()
{
	m_sceneManager = std::make_unique<ECS::SceneManager>();
}
void Engine::InitializeDirectX12()
{
	// Initialize rendering manager here as other managers depend on it
	m_sceneManager->AllocateRenderingManager();
	m_sceneManager->GetRenderingManager()->Initialize(game_window, width, height);
}
void Engine::CreateScenes()
{
	m_sceneManager->InitializeManagers(game_window, width, height, m_sceneManager->GetRenderingManager()->GetDX12().GetDevice(),
		m_sceneManager->GetRenderingManager()->GetDX12().GetCmdList(), m_sceneManager->GetRenderingManager()->GetDX12().GetDescriptorAllocator());
	m_sceneManager->LoadScene("Scene1");
	m_sceneManager->SetCurrentScene("Scene1");
	m_sceneManager->GetCurrentScene()->LoadMaterials();
	m_sceneManager->GetCurrentScene()->LoadAssets();
	m_sceneManager->GetCurrentScene()->LoadPhysics();
	m_sceneManager->SetupLights();
	m_sceneManager->GetRenderingManager()->InitializeRenderTargets(m_sceneManager->GetCurrentScene());
	m_sceneManager->GetRenderingManager()->PopulateRayTracingData(m_sceneManager->GetCurrentScene());

	m_sceneManager->GetRenderingManager()->GetDX12().SubmitCommand();

	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	m_sceneManager->GetCurrentScene()->GetCamera().PerspectiveFov(75.0f, aspectRatio, 0.1f, 1000.0f);

	m_sceneManager->GetRenderingManager()->CreateSBTs(m_sceneManager->GetCurrentScene());
}
