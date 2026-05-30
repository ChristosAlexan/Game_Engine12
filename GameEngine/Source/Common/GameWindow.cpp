#include "GameWindow.h"

GameWindow::GameWindow()
{
}

bool GameWindow::Initialize()
{
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    auto primaryDisplay = SDL_GetPrimaryDisplay();

    auto displayMode = SDL_GetCurrentDisplayMode(primaryDisplay);
    if (displayMode)
    {
        m_screenWidth = displayMode->w;
        m_screenHeight = displayMode->h;
        m_refreshRate = displayMode->refresh_rate;
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    m_window = SDL_CreateWindow(
        "SDL window",
        GetScreenWidth(), GetScreenHeight(),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN
    );

    void* hwnd = SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), "SDL.window.win32.hwnd", nullptr);
    m_nativeHwnd = reinterpret_cast<HWND>(hwnd);

    return true;
}

void GameWindow::SetScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;

    SDL_SetWindowSize(m_window, GetScreenWidth(), GetScreenHeight());
}

int GameWindow::GetRefreshRate() const
{
    return m_refreshRate;
}

HWND& GameWindow::GetWindow()
{
    return m_nativeHwnd;
}

SDL_Window* GameWindow::GetSDLWindow() const
{
	return m_window;
}

int GameWindow::GetScreenWidth() const
{
    return m_screenWidth;
}

int GameWindow::GetScreenHeight() const
{
    return m_screenHeight;
}
