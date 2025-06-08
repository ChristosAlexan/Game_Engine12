#include "DescriptorAllocator.h"
#include "COMException.h"
#include "ErrorLogger.h"
#include <stdexcept>

DescriptorAllocator::DescriptorAllocator(ID3D12Device* device, ID3D12DescriptorHeap* heap, UINT numDescriptors, bool shaderVisible)
	:m_descriptorCount(numDescriptors)
{
	m_descriptorSize = device->GetDescriptorHandleIncrementSize(heap->GetDesc().Type);
	m_cpuStart = heap->GetCPUDescriptorHandleForHeapStart();
	if (shaderVisible)
		m_gpuStart = heap->GetGPUDescriptorHandleForHeapStart();
}

DescriptorAllocator::DescriptorHandle DescriptorAllocator::Allocate()
{
	UINT index;

	// Reuse freed index if available
	if (!m_freeList.empty())
	{
		index = m_freeList.front();
		m_freeList.pop();
	}
	else
	{
		if (m_currentIndex >= m_descriptorCount)
			ErrorLogger::Log("Descriptor heap exhausted!");
		
		index = m_currentIndex++;
	}
	return { GetCPUHandle(index), GetGPUHandle(index), index };
}

UINT DescriptorAllocator::AllocateContiguous(UINT count)
{
	if (m_currentIndex + count > m_descriptorCount)
	{
		ErrorLogger::Log("Descriptor heap exhausted during contiguous allocation!");
	}

	// NOTE: This skips checking the free list (safe if you only use free list for single-slot reuse)
	UINT startIndex = m_currentIndex;
	m_currentIndex += count;
	return startIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::GetCPUHandle(UINT index) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_cpuStart;
	handle.ptr += index * m_descriptorSize;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorAllocator::GetGPUHandle(UINT index) const
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_gpuStart;
	handle.ptr += index * m_descriptorSize;
	return handle;
}

UINT DescriptorAllocator::AllocateIndex()
{
	if (m_currentIndex >= m_descriptorCount)
	{
		ErrorLogger::Log("Descriptor heap exhausted!");
		throw std::runtime_error("Descriptor heap exhausted!");
	}

	return m_currentIndex++;
}

void DescriptorAllocator::Free(UINT index)
{
	if (index >= m_descriptorCount) {
		ErrorLogger::Log("Invalid descriptor index freed!");
		return;
	}

	m_freeList.push(index);
}

void DescriptorAllocator::Reset() {
	m_currentIndex = 0;
	std::queue<UINT>().swap(m_freeList);
}

UINT DescriptorAllocator::GetCurrentIndex() const
{
	return m_currentIndex;
}
