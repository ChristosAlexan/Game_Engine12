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
		void UpdateVisibleLights(ID3D12GraphicsCommandList* cmdList, Camera& camera);

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;
	private:
		std::vector<LightComponent*> m_lights;
		std::vector<TransformComponent*> m_lightTransforms;
		std::vector<GPULight> m_gpuLights;
		UINT m_gpuLightCount = 0;

		StructuredBuffer<GPULight> m_lightBuffer;

		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{};
	};
}


