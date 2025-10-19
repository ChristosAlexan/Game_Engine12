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
	
	DescriptorAllocator(ID3D12Device* device, ID3D12DescriptorHeap* heap, UINT numDescriptors, bool shaderVisible);
	DescriptorHandle Allocate();
	UINT AllocateContiguous(UINT count);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index) const;

	UINT AllocateIndex();
	void Free(UINT index);
	void Reset();


	UINT GetCurrentIndex() const;
private:
	UINT m_currentIndex = 0;
	UINT m_descriptorCount;
	UINT m_descriptorSize;
	std::queue<UINT> m_freeList;

	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuStart{};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuStart{};

};

