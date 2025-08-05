#include "GameWindow.h"

GameWindow::GameWindow()
{
}

bool GameWindow::Initialize(int window_width, int window_height)
{
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    m_window = SDL_CreateWindow(
        "SDL window",
        window_width, window_height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    void* hwnd = SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), "SDL.window.win32.hwnd", nullptr);
    m_nativeHwnd = reinterpret_cast<HWND>(hwnd);

    return true;
}

HWND& GameWindow::GetWindow()
{
    return m_nativeHwnd;
}

SDL_Window* GameWindow::GetSDLWindow()
{
	return m_window;
}
