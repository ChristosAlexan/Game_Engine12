#pragma once
#include "DX12Includes.h"
#include <queue>

class DescriptorAllocator
{
public:
	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		UINT index;
	};

	DescriptorAllocator(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type,
		UINT numDescriptors, bool shaderVisible = true);

	DescriptorHandle Allocate();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index) const;

	UINT AllocateIndex();
	void Free(UINT index);
	void Reset();
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	D3D12_DESCRIPTOR_HEAP_TYPE m_type;
	UINT m_descriptorSize;
	UINT m_descriptorCount;
	UINT m_currentIndex = 0;
	std::queue<UINT> m_freeList;

	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuStart{};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuStart{};

};

