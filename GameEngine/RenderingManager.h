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
#include "TLASBuilder.h"

class GameWindow;
namespace ECS
{
	class Scene;
	class RenderingManager
	{
	public:
		RenderingManager();
		~RenderingManager();
		bool Initialize(GameWindow& game_window, int width, int height);
		void InitializeRenderTargets(int& width, int& height);
		void BuildTLAS(Scene* scene);
		DX12& GetDX12();
		GFXGui& GetGFXGui();
		GBuffer& GetGbuffer();
		void ResetRenderTargets();
		void SetRenderTarget(RenderTargetTexture& renderTarget, float* clearColor);
		void LightPass(Scene* scene);
		void RenderPbrPass(Camera& camera, DynamicUploadBuffer* dynamicCB);
		void RenderGbuffer(Scene* scene, entt::entity& entity, Camera& camera, DynamicUploadBuffer* dynamicCB,
			TransformComponent& transformComponent, RenderComponent& renderComponent);
		void RenderBRDF();
		void DispatchRays(Scene* scene);

		void RenderRayTracingToRenderTarget();
	private:
		void RenderLightPass(Scene* scene);


	private:
		DX12 m_dx12;
		GFXGui m_gui;
		GBuffer m_gBuffer;
		std::unique_ptr<Texture12> m_textureUAV; // Ray tracing output
		std::unique_ptr<Texture12> m_shadowsUAV; // Ray traced shadows output
 	public:

		HDR_IMAGE hdr_map1;
		CubeMap m_cubeMap1, m_irradianceMap, m_prefilterMap;
		RenderTargetTexture m_brdfMap;
		RenderTargetTexture m_raytracingMap;
		TLASBuilder m_tlasBuilder;

		bool bRenderPbrPass = true;
	};
}


