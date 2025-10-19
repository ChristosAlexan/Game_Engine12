#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

class GameWindow
{
public:
	GameWindow();

	bool Initialize(int window_width, int window_height);
	HWND& GetWindow();
	SDL_Window* GetSDLWindow();
private:
	SDL_Window* m_window;
	HWND m_nativeHwnd;
};

