#include "Engine.h"
//#include "DX12.h"
#include "GFXGui.h"
#include "RenderingManager.h"
#include "ErrorLogger.h"

using namespace DirectX;


Engine::Engine()
{
}

bool Engine::Initialize(int width, int height)
{
	this->width = width;
	this->height = height;

	timer.Start();

	if (!game_window.Initialize(width, height))
		return false;


	InitializeSceneManager();
	InitializeDirectX12();
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
	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	D3D12_RECT scissorRect = { 0, 0, width, height };

	timer.CalculateDeltaTime(dt, fps);
	timer.Restart();
	
	if (heldKeys.contains(SDLK_ESCAPE))
	{
		bStopEngine = true;
	}

	// Start rendering of a frame
	m_sceneManager->GetRenderingManager()->GetDX12().StartRenderFrame(m_sceneManager.get(), m_sceneManager->GetRenderingManager()->GetGFXGui(), camera, width, height, dt);
	// Reset all render targets before rendering
	m_sceneManager->GetRenderingManager()->ResetRenderTargets();
	// Render the scene to the geometry pass
	float clearColor[] = { 0,0,0,1 };
	m_sceneManager->GetRenderingManager()->SetRenderTarget(m_sceneManager->GetRenderingManager()->GetGbuffer().GetGbufferRenderTargetTexture(), clearColor);
	// Update current scene(animations, rendering etc.)
	m_sceneManager->Update(dt, camera, m_sceneManager->GetRenderingManager()->GetDX12().dynamicCB.get());
	// Render cube maps, irradiance, prefilter and brdf maps
	m_sceneManager->GetRenderingManager()->RenderPbrPass(camera, m_sceneManager->GetRenderingManager()->GetDX12().dynamicCB.get());

	// Reset viewport to camera
	camera.PerspectiveFov(75.0f, aspectRatio, 0.1f, 1000.0f);
	m_sceneManager->GetRenderingManager()->GetDX12().GetCmdList()->RSSetViewports(1, &viewport);
	m_sceneManager->GetRenderingManager()->GetDX12().GetCmdList()->RSSetScissorRects(1, &scissorRect);

	// Render light pass
	m_sceneManager->GetRenderingManager()->LightPass(m_sceneManager->GetCurrentScene());

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
		m_sceneManager->GetRenderingManager()->GetGFXGui().SelectEntity(m_sceneManager.get(), width, height, camera);
	}
	m_sceneManager->GetRenderingManager()->GetGFXGui().UpdateSelectedEntity(m_sceneManager.get(), width, height, camera);
	m_sceneManager->GetRenderingManager()->GetGFXGui().UpdateAllEntities(m_sceneManager.get(), width, height, camera);

	//m_sceneManager->GetCurrentScene()->GetRenderingManager()->m_cubeMap1.RenderDebug(m_sceneManager->GetCurrentScene()->GetRenderingManager()->GetDX12(), camera, 9);

	
	m_sceneManager->GetRenderingManager()->GetDX12().EndRenderFrame(m_sceneManager.get(), m_sceneManager->GetRenderingManager()->GetGFXGui(), camera, width, height, dt);

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
void Engine::CreateScenes(Camera& camera, int& width, int& height)
{
	m_sceneManager->InitializeManagers(game_window, width, height, m_sceneManager->GetRenderingManager()->GetDX12().GetDevice(), 
										m_sceneManager->GetRenderingManager()->GetDX12().GetCmdList(), m_sceneManager->GetRenderingManager()->GetDX12().GetDescriptorAllocator());
	m_sceneManager->LoadScene("Scene1", m_sceneManager->GetRenderingManager()->GetDX12().GetDevice(), m_sceneManager->GetRenderingManager()->GetDX12().GetCmdList());
	m_sceneManager->SetCurrentScene("Scene1");
	m_sceneManager->GetCurrentScene()->LoadMaterials();
	m_sceneManager->GetCurrentScene()->LoadAssets();
	m_sceneManager->SetupLights();
	m_sceneManager->GetRenderingManager()->InitializeRenderTargets(width, height);	
	m_sceneManager->GetRenderingManager()->GetDX12().SubmitCommand();

	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	camera.PerspectiveFov(75.0f, aspectRatio, 0.1f, 1000.0f);
	camera.SetPosition(0, 0, 0);
}
