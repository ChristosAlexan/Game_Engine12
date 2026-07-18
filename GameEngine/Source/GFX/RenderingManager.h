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
#include "MathHelpers.h"

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
		void InitializeRenderTargets(Scene* scene);
		void InitializeShadowTextures(Scene* scene);
		void PopulateRayTracingData(Scene* scene);
		void CreateSBTs(Scene* scene);
		void BuildTLAS(Scene* scene);
		void RefitBLAS(Scene* scene);
		DX12& GetDX12();
		GFXGui& GetGFXGui();
		GBuffer& GetGbuffer();
		void ResetRenderTargets();
		void SetRenderTarget(RenderTargetTexture& renderTarget, float* clearColor);
		void RenderPbrMaps(Camera& camera);
		void RenderGbuffer(Scene* scene, entt::entity& entity, TransformComponent& transformComponent, RenderComponent& renderComponent);

		void CullAndBucketEntities(Scene* scene, const Frustum& frustum);

		void RenderGbufferInstanced(Scene* scene, const Frustum& frustum);
		void RenderBRDF();
		void DispatchRays(Scene* scene);
		void CalculateCompute(Scene* scene);
		void UpdatePBR(Scene* scene);
		void DebugDraw(Scene* scene);
		void FXAA();
		void SetGbufferRenderTarget();
		DirectX::XMFLOAT3 GetAmbientColor() const;
		float GetExposure() const;
		float GetGamma() const;
	private:
		void RayTracedShadows(Scene* scene);
		void RayTracedReflections(Scene* scene);
		void RayTracedAO(Scene* scene);
		void RenderLightPass(Scene* scene);


	private:
		int m_screenWidth, m_screenHeight;
		DX12 m_dx12;
		GFXGui m_gui;
		GBuffer m_gBuffer;
		Texture12* m_shadowsTexture = nullptr; // Ray traced shadows output
		std::unique_ptr<RenderTargetTexture> m_lightPassRenderTarget; // Light pass output
		std::unique_ptr<Texture12> m_reflectionsTexture; // Ray traced reflections output
		std::unique_ptr<Texture12> m_AOTexture; // Ray traced ambient occlusion output
		std::unique_ptr<Texture12> m_bindlessAlbedoTextures; // Bindless albedo textures for rt reflections
		std::unique_ptr<RenderTargetTexture> m_brdfMap;
		std::unique_ptr<RTEntityHandle> m_rtEntityHandle;

		HDR_IMAGE hdr_map1;
		
		TLASBuilder m_tlasBuilder;
		ECS::TLAS m_tlas;
		bool bRenderPbrMaps = true;

		ComputeSkinning m_computeSkinning;

		// Ray Tracing data
		UINT m_totalEntities = 0;
		std::unique_ptr<std::map<uint32_t, std::shared_ptr<Texture12>>> m_textureMapping;
		std::vector<ECS::RTVertexData> rt_vertexData;
		std::vector<ECS::RTIndexData> rt_indexData;
		std::vector<ECS::RTMeshDataOffsets> rt_meshDataOffsests;
		std::unordered_map<BatchKey, BatchEntry, BatchKeyHash> multiBatches;
		std::vector<DirectX::XMFLOAT4X4> m_visibleInstanceTransforms;

	public:
		std::vector<entt::entity> individualDraws;
		DirectX::XMFLOAT3 m_ambientColor;
		float m_exposure;
		float m_gamma;
		bool m_bEnableDebugDraw = false;
		CubeMap m_cubeMap1, m_irradianceMap, m_prefilterMap;
		CB_AO_Data aoData;

		CB_SHADER_FXAA fxaaCB;
	};
}


