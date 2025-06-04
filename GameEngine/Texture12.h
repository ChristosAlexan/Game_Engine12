#pragma once
#include <d3d12.h>
#include <DirectXTex.h>
#include "DescriptorAllocator.h"

class Texture12
{
public:
	Texture12();
	void LoadFromFile(const std::string& filename, 
		ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator& srvAllocator);
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
	std::string debugName;
};

