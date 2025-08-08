#include "TLASBuilder.h"
#include "TransformECS.h"
#include "RenderingECS.h"
#include "RenderingManager.h"
#include "Scene.h"
#include "MathHelpers.h"

TLASBuilder::TLASBuilder()
{
}

void TLASBuilder::Build(ECS::Scene* scene)
{
	m_instanceDescs.clear();
	m_tlasBuffer.Reset();
	m_instanceBuffer.Reset();
	m_scratchBuffer.Reset();

	auto group = scene->GetRegistry().group<>(entt::get<ECS::TransformComponent, ECS::RenderComponent>);
	for (auto [entity, transformComponent, renderComponent] : group.each())
	{
		const auto& blas = renderComponent.mesh->blas;
	
		if (!blas)
			continue;

		D3D12_RAYTRACING_INSTANCE_DESC instance = {};
		instance.AccelerationStructure = blas->result->GetGPUVirtualAddress();
		instance.InstanceID = static_cast<UINT>(entity);
		instance.InstanceMask = 0xFF;
		instance.InstanceContributionToHitGroupIndex = m_instanceDescs.size();
		instance.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(transformComponent.worldMatrix);

		for (int i = 0; i < 3; ++i)
		{
			instance.Transform[i][0] = worldMatrix.r[i].m128_f32[0];
			instance.Transform[i][1] = worldMatrix.r[i].m128_f32[1];
			instance.Transform[i][2] = worldMatrix.r[i].m128_f32[2];
			instance.Transform[i][3] = worldMatrix.r[i].m128_f32[3];
		}
		m_instanceDescs.push_back(instance);
	}

	BuildRAS(scene->GetRenderingManager()->GetDX12());
}

void TLASBuilder::BuildRAS(DX12& dx12)
{
	// Upload m_instanceDescs to a GPU buffer
	const UINT instanceDescsSize = static_cast<UINT>(m_instanceDescs.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
	m_instanceBuffer = dx12.CreateRaytracingInstanceUploadBuffer(instanceDescsSize, m_instanceDescs.data());

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs.NumDescs = (UINT)m_instanceDescs.size();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
	dx12.GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);

	// Create result buffer for the TLAS
	m_tlasBuffer = dx12.CreateRayTracingBuffer(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
	m_scratchBuffer = dx12.CreateRayTracingBuffer(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	
	// Build the TLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuildDesc = {};
	tlasBuildDesc.Inputs = inputs;
	tlasBuildDesc.Inputs.InstanceDescs = m_instanceBuffer->GetGPUVirtualAddress();
	tlasBuildDesc.ScratchAccelerationStructureData = m_scratchBuffer->GetGPUVirtualAddress();
	tlasBuildDesc.DestAccelerationStructureData = m_tlasBuffer->GetGPUVirtualAddress();

	dx12.GetCmdList()->BuildRaytracingAccelerationStructure(&tlasBuildDesc, 0, nullptr);
}

std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& TLASBuilder::GetInstances()
{
	return m_instanceDescs;
}
