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
#include "Physics/PhysicsDebugDraw.h"

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
		void ReBuildBLAS(Scene* scene);
		DX12& GetDX12();
		GFXGui& GetGFXGui();
		GBuffer& GetGbuffer();
		void ResetRenderTargets();
		void SetRenderTarget(RenderTargetTexture& renderTarget, float* clearColor);
		void RenderPbrMaps(Camera& camera);
		void RenderGbuffer(Scene* scene, entt::entity& entity, Camera& camera,
			TransformComponent& transformComponent, RenderComponent& renderComponent);
		void RenderBRDF();
		void DispatchRays(Scene* scene);
		void RenderRayTracingToRenderTarget();
		void CalculateCompute(Scene* scene);
		void UpdatePBR(Scene* scene, Camera& camera);
		void DebugDraw(Scene* scene, Camera& camera);
		void SetGbufferRenderTarget();
		DirectX::XMFLOAT3 GetAmbientColor() const;
		float GetExposure() const;
		float GetGamma() const;
	private:
		void RenderLightPass(Scene* scene);


	private:
		DX12 m_dx12;
		GFXGui m_gui;
		GBuffer m_gBuffer;
		std::unique_ptr<Texture12> m_textureUAV; // Ray tracing output
		std::unique_ptr<Texture12> m_shadowsUAV; // Ray traced shadows output
		std::unique_ptr<RenderTargetTexture> m_brdfMap, m_raytracingMap;
		HDR_IMAGE hdr_map1;
		CubeMap m_cubeMap1, m_irradianceMap, m_prefilterMap;

		TLASBuilder m_tlasBuilder;
		DirectX::XMFLOAT3 m_ambientColor;
		float m_exposure;
		float m_gamma;
		ECS::TLAS m_tlas;
		bool bRenderPbrPass = true;
 	public:
		bool m_bEnableDebugDraw = false;
	};
}


