#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DirectXMath.h>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

class GameWindow
{
public:
	GameWindow();

	bool Initialize();
	HWND& GetWindow();
	SDL_Window* GetSDLWindow() const;
	DirectX::XMFLOAT2 GetScreenResoulution() const;
	int GetRefreshRate() const;
	
private:
	SDL_Window* m_window;
	HWND m_nativeHwnd;
	DirectX::XMFLOAT2 m_screenResolution;
	int m_refreshRate;
};

