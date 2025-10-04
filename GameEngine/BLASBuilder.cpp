#include "BLASBuilder.h"
#include "COMException.h"
#include "Vertex.h"


BLASBuilder::BLASBuilder()
{
}

ECS::BLAS BLASBuilder::Build(ID3D12Device5* device, ID3D12GraphicsCommandList5* cmdList, 
	D3D12_GPU_VIRTUAL_ADDRESS vertexBuffer, const UINT vertexCount, UINT vertexStride, 
	D3D12_GPU_VIRTUAL_ADDRESS indexBuffer, const UINT indexCount, DXGI_FORMAT indexFormat, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS inputFlags)
{
	HRESULT hr;
	ECS::BLAS blas;

	// Geometry description
	D3D12_RAYTRACING_GEOMETRY_DESC geometry = {};
	geometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometry.Triangles.VertexBuffer.StartAddress = vertexBuffer;
	geometry.Triangles.VertexBuffer.StrideInBytes = vertexStride;
	geometry.Triangles.Transform3x4 = 0;
	geometry.Triangles.VertexCount = vertexCount;
	geometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometry.Triangles.IndexBuffer = indexBuffer;
	geometry.Triangles.IndexCount = indexCount;
	geometry.Triangles.IndexFormat = indexFormat;
	geometry.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	
	// Prebuild info
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &geometry;
	inputs.Flags = inputFlags;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);
	
	// Create scratch buffer
	D3D12_RESOURCE_DESC scratchDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuild.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	Microsoft::WRL::ComPtr<ID3D12Resource> scratch;
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	hr = device->CreateCommittedResource(&heapProps,
		D3D12_HEAP_FLAG_NONE, &scratchDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&scratch));
	COM_ERROR_IF_FAILED(hr, "Failed to create scratch buffer!");

	// Create result buffer
	D3D12_RESOURCE_DESC resultDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuild.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	Microsoft::WRL::ComPtr<ID3D12Resource> result;
	hr = device->CreateCommittedResource(&heapProps,
		D3D12_HEAP_FLAG_NONE, &resultDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&result));
	COM_ERROR_IF_FAILED(hr, "Failed to create result buffer!");

	// Build command
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
	buildDesc.Inputs = inputs;
	buildDesc.DestAccelerationStructureData = result->GetGPUVirtualAddress();
	buildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();

	cmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(result.Get());
	cmdList->ResourceBarrier(1, &barrier);
	blas.result = result;
	blas.scratch = scratch;
	blas.prebuild = prebuild;
	blas.inputs = inputs;
	blas.geometry = geometry;

	return blas;
}

void BLASBuilder::ReBuild(ID3D12Device5* device, ID3D12GraphicsCommandList5* cmdList, ECS::RenderComponent& renderComponent)
{
	if (!renderComponent.blas)
		return;

	auto inputs = renderComponent.blas->inputs;
	inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	
	auto geometry = renderComponent.blas->geometry;
	geometry.Triangles.VertexBuffer.StartAddress = renderComponent.skinningOutData.skinningVertexBufferFinalTransform.GetGPUAddress();
	geometry.Triangles.VertexBuffer.StrideInBytes = 64; // 64 bytes to properly offset to the next data set. (position, normal, tangent, binormal) each is 16 bytes with padding
	geometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc{};
	desc.Inputs = inputs;
	desc.Inputs.pGeometryDescs = &geometry;
	
	desc.SourceAccelerationStructureData = renderComponent.blas->result->GetGPUVirtualAddress();
	desc.DestAccelerationStructureData = renderComponent.blas->result->GetGPUVirtualAddress();
	desc.ScratchAccelerationStructureData = renderComponent.blas->scratch->GetGPUVirtualAddress();
	cmdList->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
	
	auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(renderComponent.blas->result.Get());
	cmdList->ResourceBarrier(1, &barrier);
}
