#pragma once
#include "RayTraceData.h"

class BLASBuilder
{
public:
	BLASBuilder();
	ECS::BLAS Build(ID3D12Device5* device, ID3D12GraphicsCommandList5* cmdList,
		D3D12_GPU_VIRTUAL_ADDRESS vertexBuffer, const UINT vertexCount, UINT vertexStride,
		D3D12_GPU_VIRTUAL_ADDRESS indexBuffer, const UINT indexCount, DXGI_FORMAT indexFormat);
};

