#pragma once
#include "DX12Includes.h"
#include "COMException.h"

template<typename T>
class StructuredBuffer
{
public:
	void Initialize(ID3D12Device* device, UINT elementCount)
	{
		HRESULT hr;
		m_elementCount = elementCount;
		m_stride = sizeof(T);
		UINT bufferSize = m_stride * m_elementCount;

		// Create GPU buffer (default heap)
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE);

		hr = device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE,
			&bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_gpuBuffer));
		COM_ERROR_IF_FAILED(hr, "Failed to create gpu buffer commited resource!");

		// Create upload buffer
		CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		hr = device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE,
			&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadBuffer));
		COM_ERROR_IF_FAILED(hr, "Failed to create upload buffer commited resource!");

		// Map upload buffer
		CD3DX12_RANGE readRange(0, 0);
		hr = m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedUpload));
		COM_ERROR_IF_FAILED(hr, "Failed to map upload buffer!");
	}

	void UploadData(ID3D12GraphicsCommandList* cmdList, const std::vector<T>& data)
	{
		assert(data.size() <= m_elementCount);
		memcpy(m_mappedUpload, data.data(), data.size() * sizeof(T));
		cmdList->CopyBufferRegion(m_gpuBuffer.Get(), 0, m_uploadBuffer.Get(), 0, data.size() * sizeof(T));
	}

	void CreateSRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = m_elementCount;
		srvDesc.Buffer.StructureByteStride = m_stride;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		device->CreateShaderResourceView(m_gpuBuffer.Get(), &srvDesc, srvHandle);
	}

	ID3D12Resource* GetResource() const 
	{
		return m_gpuBuffer.Get(); 
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const 
	{ 
		return m_gpuBuffer->GetGPUVirtualAddress(); 
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_gpuBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
	UINT8* m_mappedUpload = nullptr;

	UINT m_elementCount = 0;
	UINT m_stride = 0;
};