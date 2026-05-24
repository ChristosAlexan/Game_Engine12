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
	void SetScreenSize(int width, int height);
	HWND& GetWindow();
	SDL_Window* GetSDLWindow() const;
	int GetScreenWidth() const;
	int GetScreenHeight() const;
	int GetRefreshRate() const;
	
private:
	SDL_Window* m_window;
	HWND m_nativeHwnd;
	int m_screenWidth, m_screenHeight;
	int m_refreshRate;
};

