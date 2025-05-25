#pragma once
#include <fmod/fmod_studio.hpp>
#include <fmod/fmod.hpp>
#include <DirectXMath.h>
#include <future>

class SoundComponent
{
public:
	SoundComponent();
	~SoundComponent();
	bool Initialize(const char* filePath, bool is3D);
	bool Play();
	void Update();
	void Async_Play();
	void Async_Update();

	void UpdatePos(const DirectX::XMFLOAT3& destPos, const DirectX::XMVECTOR& forwardVec, const DirectX::XMVECTOR& upVec);


	FMOD_VECTOR position;
	FMOD::Studio::System* system;
	FMOD::System* mpSystem;
	int mnNextChannelId;
	FMOD::Sound* sound = nullptr;
	FMOD::Channel* channel = nullptr;



	std::future<void> async_Update;
	std::future<bool> async_Play;
private:
	bool b3D = false;
};

