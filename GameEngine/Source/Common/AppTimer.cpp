#include "AppTimer.h"
#include <Windows.h>
#include <string>


AppTimer::AppTimer()
	:start(std::chrono::high_resolution_clock::now()), stop(std::chrono::high_resolution_clock::now())
{
}

double AppTimer::GetMilisecondsElapsed()
{
	if (isRunning)
	{
		auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start);
		return elapsed.count();
	}
	else
	{
		auto elapsed = std::chrono::duration<double, std::milli>(stop - start);
		return elapsed.count();
	}
}
int AppTimer::GetSecondsElapsed()
{
	return (int)(GetMilisecondsElapsed() / 1000);
}

void AppTimer::Restart()
{
	isRunning = true;
	start = std::chrono::high_resolution_clock::now();
}

bool AppTimer::Stop()
{
	if (!isRunning)
	{
		return false;
	}
	else
	{
		stop = std::chrono::high_resolution_clock::now();
		isRunning = false;
		return true;
	}
}

bool AppTimer::Start()
{
	if (isRunning)
	{
		return false;
	}
	else
	{
		start = std::chrono::high_resolution_clock::now();
		isRunning = true;
		return true;
	}
}

void AppTimer::StartSeconds()
{

	if (!isRunning)
	{
		start = std::chrono::steady_clock::now();
		isRunning = true;
	}

}

float AppTimer::GetMilliseconds()
{
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	return elapsed.count();
}

void AppTimer::CalculateDeltaTime(float& deltaTime, float& fps)
{
    auto now = std::chrono::high_resolution_clock::now();

    const float minDelta = 1e-6f;
    deltaTime = std::max(std::chrono::duration<float>(now - lastTime).count(), minDelta);
    lastTime = now;

    fps = 1.0f / deltaTime;
}

float AppTimer::GetAverageFPS(Metrics& metrics)
{
	static float fpsBuffer[60] = {};  // store last 60 FPS values (1 second @ 60 FPS)
	static int fpsIndex = 0;

	float currentFps = 1.0f / metrics.dt; // or your delta time
	fpsBuffer[fpsIndex] = currentFps;
	fpsIndex = (fpsIndex + 1) % ARRAYSIZE(fpsBuffer);

	// Compute average
	metrics.avgFps = 0.0f;
	for (float v : fpsBuffer)
		metrics.avgFps += v;
	metrics.avgFps /= ARRAYSIZE(fpsBuffer);

	return metrics.avgFps;
}
