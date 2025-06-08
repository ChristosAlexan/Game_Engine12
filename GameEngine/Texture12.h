#pragma once
#include <d3d12.h>
#include <DirectXTex.h>
#include "DescriptorAllocator.h"

class Texture12
{
public:
	Texture12();
	void LoadFromFile(const std::string& filename, 
		ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator);
	
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;

	void BindTexture(UINT index, ID3D12GraphicsCommandList* cmdList);

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{};
	std::string m_debugName;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
};

