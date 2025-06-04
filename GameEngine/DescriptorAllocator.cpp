#include "DescriptorAllocator.h"
#include "COMException.h"
#include "ErrorLogger.h"
#include <stdexcept>

DescriptorAllocator::DescriptorAllocator(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible)
	:m_type(type), m_descriptorCount(numDescriptors)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap));
	COM_ERROR_IF_FAILED(hr, "Failed to create descriptor heap!");

	m_descriptorSize = device->GetDescriptorHandleIncrementSize(type);
	m_cpuStart = m_heap->GetCPUDescriptorHandleForHeapStart();
	if (shaderVisible)
		m_gpuStart = m_heap->GetGPUDescriptorHandleForHeapStart();
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
	return { GetCPUHandle(m_currentIndex), GetGPUHandle(m_currentIndex), m_currentIndex };
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