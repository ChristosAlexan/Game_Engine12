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
	m_instanceBuffer.Reset();

	auto group = scene->GetRegistry().group<>(entt::get<ECS::TransformComponent, ECS::RenderComponent>);
	for (auto [entity, transformComponent, renderComponent] : group.each())
	{
		std::shared_ptr<ECS::BLAS> blas;

		if(renderComponent.meshType == ECS::STATIC_MESH)
			blas = renderComponent.mesh->staticBlas;
		else if (renderComponent.meshType == ECS::MESH_TYPE::SKELETAL_MESH)
			blas = renderComponent.mesh->skinnedBlas;
		else
			continue;

		if (!blas)
			continue;

		D3D12_RAYTRACING_INSTANCE_DESC instance = {};
		instance.AccelerationStructure = blas->result->GetGPUVirtualAddress();
		instance.InstanceID = static_cast<UINT>(entity);
		instance.InstanceMask = 0xFF;
		instance.InstanceContributionToHitGroupIndex = m_instanceDescs.size();
		instance.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

		DirectX::XMMATRIX worldMatrix = transformComponent.worldMatrix;
		DirectX::XMFLOAT3X4 m34;
		DirectX::XMStoreFloat3x4(&m34, worldMatrix);
		memcpy(instance.Transform, &m34, sizeof(m34));
		
		m_instanceDescs.push_back(instance);
	}

	BuildRAS(scene->GetRenderingManager()->GetDX12(), bRefit);

	bRefit = true;
}

void TLASBuilder::BuildRAS(DX12& dx12, bool bRefit)
{
	// Upload m_instanceDescs to a GPU buffer
	const UINT instanceDescsSize = static_cast<UINT>(m_instanceDescs.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
	m_instanceBuffer = dx12.CreateRaytracingInstanceUploadBuffer(instanceDescsSize, m_instanceDescs.data());

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = (UINT)m_instanceDescs.size();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuildDesc = {};

	if (!bRefit)
	{
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
		dx12.GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
		// Create result buffer for the TLAS
		m_tlasBuffer = dx12.CreateRayTracingBuffer(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
		m_scratchBuffer = dx12.CreateRayTracingBuffer(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	

		tlasBuildDesc.Inputs = inputs;
		tlasBuildDesc.Inputs.InstanceDescs = m_instanceBuffer->GetGPUVirtualAddress();
		tlasBuildDesc.ScratchAccelerationStructureData = m_scratchBuffer->GetGPUVirtualAddress();
		tlasBuildDesc.DestAccelerationStructureData = m_tlasBuffer->GetGPUVirtualAddress();
	}
	else
	{
		inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	
		tlasBuildDesc.Inputs = inputs;
		tlasBuildDesc.SourceAccelerationStructureData = m_tlasBuffer->GetGPUVirtualAddress();
		tlasBuildDesc.Inputs.InstanceDescs = m_instanceBuffer->GetGPUVirtualAddress();
		tlasBuildDesc.ScratchAccelerationStructureData = m_scratchBuffer->GetGPUVirtualAddress();
		tlasBuildDesc.DestAccelerationStructureData = m_tlasBuffer->GetGPUVirtualAddress();
	}

	dx12.GetCmdList()->BuildRaytracingAccelerationStructure(&tlasBuildDesc, 0, nullptr);
}

std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& TLASBuilder::GetInstances()
{
	return m_instanceDescs;
}
