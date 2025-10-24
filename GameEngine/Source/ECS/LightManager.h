#pragma once
#include "LightECS.h"
#include "TransformECS.h"
#include <vector>
#include "StructuredBuffer.h"

class Camera;
namespace ECS
{
	class Scene;
	class LightManager
	{
	public:
		LightManager();
		void Initialize(Scene* scene);
		void AccumulateLights(Scene* scene);
		std::vector<LightComponent*>& GetLights();
		std::vector<GPULight>& GetGPULights();
		std::vector<TransformComponent*>& GetTransforms();
		ID3D12Resource* GetResource() const;
		void UpdateVisibleLights(ID3D12GraphicsCommandList* cmdList, Camera& camera);

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;

		ResourceWrapper* GetShadowsResourceWrapper() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetShadowsUavGPUHandle() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetShadowsSrvGPUHandle() const;

	private:
		std::vector<LightComponent*> m_lights;
		std::vector<TransformComponent*> m_lightTransforms;
		std::vector<GPULight> m_gpuLights;
		std::vector<GPUShadows> m_gpuShadows;
		UINT m_gpuLightCount = 0;

		StructuredBuffer<GPULight> m_lightBuffer;
		StructuredBuffer<GPUShadows> m_ShadowsBuffer;

		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{};

		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuShadowSrvHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuShadowSrvHandle{};
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuShadowUavHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuShadowUavHandle{};
	};
}


