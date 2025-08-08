
#include"Engine.h"
#include<Windows.h>
#include "ErrorLogger.h"

void EnableConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	freopen("CONIN$", "r", stdin);

	std::ios::sync_with_stdio(); // Sync iostream with C stdio
}

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, INT nCmdShow)
{
#ifdef _DEBUG
	EnableConsole();
	LoadLibraryA("C:\\Program Files\\Microsoft PIX\\2507.11\\WinPixGpuCapturer.dll");
#endif

	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to call CoInitialize.");
		return -1;
	}

	const int w = 1600;
	const int h = 900;
	Engine engine;
	if (engine.Initialize(w, h))
	{
		while (engine.StopEngine() != true)
		{
			engine.Update(w, h);
		}
	}
	ClipCursor(NULL);
	SDL_Quit();

	return 0;
}