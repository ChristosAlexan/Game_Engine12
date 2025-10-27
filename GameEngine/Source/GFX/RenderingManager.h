#pragma once
#include "DX12.h"
#include "Camera.h"
#include "ComputeSkinning.h"
#include "DynamicUploadBuffer.h"
#include "RenderingECS.h"
#include "ModelData.h"
#include "TransformECS.h"
#include "RenderTargetTexture.h"
#include "GBuffer.h"
#include "HDR_IMAGE.h"
#include "CubeMap.h"
#include "TLASBuilder.h"
#include "PhysicsDebugDraw.h"

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
		void InitializeShadowTextures(Scene* scene);
		void BuildTLAS(Scene* scene);
		void RefitBLAS(Scene* scene);
		DX12& GetDX12();
		GFXGui& GetGFXGui();
		GBuffer& GetGbuffer();
		void ResetRenderTargets();
		void SetRenderTarget(RenderTargetTexture& renderTarget, float* clearColor);
		void RenderPbrMaps(Camera& camera);
		void RenderGbuffer(Scene* scene, entt::entity& entity,
			TransformComponent& transformComponent, RenderComponent& renderComponent);
		void RenderBRDF();
		void DispatchRays(Scene* scene);
		void CalculateCompute(Scene* scene);
		void UpdatePBR(Scene* scene);
		void DebugDraw(Scene* scene);
		void SetGbufferRenderTarget();
		DirectX::XMFLOAT3 GetAmbientColor() const;
		float GetExposure() const;
		float GetGamma() const;
	private:
		void RayTracedShadows(Scene* scene);
		void RayTracedReflections(Scene* scene);
		void RenderLightPass(Scene* scene);


	private:
		DX12 m_dx12;
		GFXGui m_gui;
		GBuffer m_gBuffer;
		Texture12* m_shadowsTexture = nullptr; // Ray traced shadows output
		std::unique_ptr<Texture12> m_reflectionsTexture; // Ray traced reflections output
		std::unique_ptr<RenderTargetTexture> m_brdfMap;
		HDR_IMAGE hdr_map1;
		CubeMap m_cubeMap1, m_irradianceMap, m_prefilterMap;
		TLASBuilder m_tlasBuilder;
		ECS::TLAS m_tlas;
		bool bRenderPbrPass = true;

		ComputeSkinning m_computeSkinning;
 	public:
		DirectX::XMFLOAT3 m_ambientColor;
		float m_exposure;
		float m_gamma;
		bool m_bEnableDebugDraw = false;
	};
}


