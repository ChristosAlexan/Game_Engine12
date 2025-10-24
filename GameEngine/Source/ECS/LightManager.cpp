#include "LightManager.h"
#include "Scene.h"
#include "Camera.h"
#include "RenderingManager.h"

namespace ECS
{
	LightManager::LightManager()
	{

	}

	void LightManager::Initialize(Scene* scene)
	{
		AccumulateLights(scene);
		m_gpuLights.resize(m_lights.size()); // resize to maximum number of lights
		m_gpuShadows.resize(m_lights.size()); // resize to maximum number of lights

		m_lightBuffer.Initialize(scene->GetRenderingManager()->GetDX12().GetDevice(), m_gpuLights.size());
		m_ShadowsBuffer.Initialize(scene->GetRenderingManager()->GetDX12().GetDevice(), m_gpuShadows.size(), true);

		DescriptorAllocator::DescriptorHandle allocator = scene->GetRenderingManager()->GetDX12().GetDescriptorAllocator()->Allocate();
	
		m_cpuHandle = allocator.cpuHandle;
		m_gpuHandle = allocator.gpuHandle;
		
		m_lightBuffer.CreateSRV(scene->GetRenderingManager()->GetDX12().GetDevice(), m_cpuHandle);

		// Allocate shadows srv handles
		allocator = scene->GetRenderingManager()->GetDX12().GetDescriptorAllocator()->Allocate();
		m_cpuShadowSrvHandle = allocator.cpuHandle;
		m_gpuShadowSrvHandle = allocator.gpuHandle;
		m_ShadowsBuffer.CreateSRV(scene->GetRenderingManager()->GetDX12().GetDevice(), m_cpuShadowSrvHandle);

		// Allocate shadows uav handles
		allocator = scene->GetRenderingManager()->GetDX12().GetDescriptorAllocator()->Allocate();
		m_cpuShadowUavHandle = allocator.cpuHandle;
		m_gpuShadowUavHandle = allocator.gpuHandle;
		m_ShadowsBuffer.CreateUAV(scene->GetRenderingManager()->GetDX12().GetDevice(), m_cpuShadowUavHandle);

		m_ShadowsBuffer.GetResource()->TransitionState(scene->GetRenderingManager()->GetDX12().GetCmdList(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	void LightManager::AccumulateLights(Scene* scene)
	{
		auto group = scene->GetRegistry().group<>(entt::get<LightComponent, TransformComponent>);
		for (auto [entity, lightComponent, transformComponent] : group.each())
		{
			m_lights.push_back(&lightComponent);
			m_lightTransforms.push_back(&transformComponent);
		}
	}

	std::vector<LightComponent*>& LightManager::GetLights()
	{
		return m_lights;
	}

	std::vector<GPULight>& LightManager::GetGPULights()
	{
		return m_gpuLights;
	}

	std::vector<TransformComponent*>& LightManager::GetTransforms()
	{
		return m_lightTransforms;
	}

	ID3D12Resource* LightManager::GetResource() const
	{
		return m_lightBuffer.GetResource()->GetResource();
	}

	void LightManager::UpdateVisibleLights(ID3D12GraphicsCommandList* cmdList, Camera& camera)
	{
		// Transition to copy dest
		m_lightBuffer.GetResource()->TransitionState(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);
		for (int i = 0; i < m_lights.size(); ++i)
		{
			auto& worldMatrix = m_lightTransforms[i]->worldMatrix;
	
			GPULight light{};
			light.color = m_lights[i]->color;
			light.radius = m_lights[i]->radius;
			light.strength = m_lights[i]->strength;
			light.cutoff = m_lights[i]->cutoff;

			light.direction = m_lightTransforms[i]->rotation;
			light.lightType = (uint32_t)m_lights[i]->lightType;
			light.position = m_lightTransforms[i]->position;
		
			light.padding = DirectX::XMFLOAT2(0.0f, 0.0f);
			m_gpuLights[i] = light;

			m_lightBuffer.UploadData(cmdList, m_gpuLights);
			
		}
		// Transition to non pixel/pixel
		m_lightBuffer.GetResource()->TransitionState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cmdList->SetGraphicsRootDescriptorTable(
			7, // Root parameter light's structured buffer index is 7
			m_gpuHandle
		);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE LightManager::GetGPUHandle() const
	{
		return m_gpuHandle;
	}

	ResourceWrapper* LightManager::GetShadowsResourceWrapper() const
	{
		return m_ShadowsBuffer.GetResource();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE LightManager::GetShadowsUavGPUHandle() const
	{
		return m_gpuShadowUavHandle;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE LightManager::GetShadowsSrvGPUHandle() const
	{
		return m_gpuShadowSrvHandle;
	}
}