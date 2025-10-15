#pragma once
#include "RayTraceData.h"
#include "MeshData.h"
#include "RenderingECS.h"

class BLASBuilder
{
public:
	BLASBuilder();
	ECS::BLAS Build(ID3D12Device5* device, ID3D12GraphicsCommandList5* cmdList,
		D3D12_GPU_VIRTUAL_ADDRESS vertexBuffer, const UINT vertexCount, UINT vertexStride,
		D3D12_GPU_VIRTUAL_ADDRESS indexBuffer, const UINT indexCount, DXGI_FORMAT indexFormat, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS inputFlags);
	void Refit(ID3D12Device5* device, ID3D12GraphicsCommandList5* cmdList, ECS::RenderComponent& renderComponent); // Blas refit: used to refit skinned meshes
};

