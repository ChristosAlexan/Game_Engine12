#pragma once

#include "DX12Includes.h"
#include <stdexcept>
#include "ResourceWrapper.h"

class IndexBuffer12
{
private:
	ResourceWrapper* m_indexBuffer = nullptr;
	ResourceWrapper* m_indexUploadBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_ibView = {};
public:
	IndexBuffer12() 
	{
		m_indexBuffer = new ResourceWrapper(D3D12_RESOURCE_STATE_COPY_DEST);
		m_indexUploadBuffer = new ResourceWrapper(D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	HRESULT Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const uint32_t* data, UINT indexCount)
	{
		HRESULT hr;

		UINT ibSize = sizeof(uint32_t) * indexCount;
		// Create default heap
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC ibResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(ibSize);
		hr = device->CreateCommittedResource(
			&defaultHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&ibResourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(m_indexBuffer->ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create default heap");
		}

		// Create upload heap
		CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		hr = device->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&ibResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_indexUploadBuffer->ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create upload heap");
		}

		// Upload data
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = reinterpret_cast<const BYTE*>(data);
		indexData.RowPitch = ibSize;
		indexData.SlicePitch = ibSize;

		UpdateSubresources(commandList, m_indexBuffer->GetResource(), m_indexUploadBuffer->GetResource(), 0, 0, 1, &indexData);
		
		m_indexBuffer->TransitionState(commandList, D3D12_RESOURCE_STATE_INDEX_BUFFER);

		// Fill out IB view
		m_ibView.BufferLocation = m_indexBuffer->GetResource()->GetGPUVirtualAddress();
		m_ibView.SizeInBytes = ibSize;
		m_ibView.Format = DXGI_FORMAT_R32_UINT;

		return S_OK;
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetIndexBufferVirtualAddress() const
	{
		return m_indexBuffer->GetResource()->GetGPUVirtualAddress();
	}

	ResourceWrapper* GetResource() const
	{
		return m_indexBuffer;
	}

	D3D12_INDEX_BUFFER_VIEW GetBufferView() const
	{
		return m_ibView;
	}

	const D3D12_INDEX_BUFFER_VIEW* GetBufferViewPtr() const
	{
		return &m_ibView;
	}
};
