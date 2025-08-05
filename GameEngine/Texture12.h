#pragma once
#include <d3d12.h>
#include <DirectXTex.h>
#include "DescriptorAllocator.h"

class Texture12
{
public:
	enum TEXTURE_FORMAT
	{
		AUTO = 0,
		WIC_FILE = 1,
		DDS_FILE = 2
	};

	Texture12();
	~Texture12();
	void LoadFromFileWIC(const std::string& filename, 
		ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* descriptorAllocator);
	void LoadFromFileDDS(const std::string& filename,
		ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* descriptorAllocator);
	void LoadFromFileHDR(const std::string& filename, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* descriptorAllocator);
	
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleUAV() const;
	void TransitionToRTV(ID3D12GraphicsCommandList* cmdList);
	void TransitionToSRV(ID3D12GraphicsCommandList* cmdList);
	void CreateTextureUAV(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* descriptorAllocator, const UINT width, const UINT height);
	void Reset(ID3D12GraphicsCommandList* cmdList);
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource, m_srvResource;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{}, m_cpuHandleUAV{};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{}, m_gpuHandleUAV{};
	std::string m_debugName;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
	UINT m_width = 0;
	UINT m_height = 0;
};

