#pragma once
#include "DX12.h"
#include "Camera.h"
#include "DynamicUploadBuffer.h"
#include "RenderingECS.h"
#include "ModelData.h"
#include "TransformECS.h"
#include "RenderTargetTexture.h"
#include "GBuffer.h"
#include "HDR_IMAGE.h"
#include "CubeMap.h"

class GameWindow;
namespace ECS
{
	class Scene;
	class RenderingManager
	{
	public:
		RenderingManager();
		bool Initialize(GameWindow& game_window, int width, int height);
		void InitializeRenderTargets(int& width, int& height);
		DX12& GetDX12();
		GFXGui& GetGFXGui();
		GBuffer& GetGbuffer();
		void ResetRenderTargets();
		void SetRenderTarget(RenderTargetTexture& renderTarget, float* clearColor);
		void RenderGbufferFullscreen();
		void RenderCubeMap(Camera& camera, DynamicUploadBuffer* dynamicCB, CubeMap& cubeMap);
		void Render(Scene* scene, entt::entity& entity, Camera& camera, DynamicUploadBuffer* dynamicCB,
			TransformComponent& transformComponent, RenderComponent& renderComponent);

		void RenderFullScreenQuad(RenderTargetTexture& renderTexture, UINT rootParameterIndex);

	private:
		DX12 m_dx12;
		GFXGui m_gui;
		GBuffer m_gBuffer;
	public:
		CubeMap m_cubeMap1;
	};
}


