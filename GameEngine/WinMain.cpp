#include"Engine.h"
#include<Windows.h>
#include "ErrorLogger.h"

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, INT nCmdShow)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to call CoInitialize.");
		return -1;
	}

	const int w = 1600;
	const int h = 900;
	Engine engine;
	if (engine.Initialize("DXEngine", "Window", w, h))
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