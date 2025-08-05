#pragma once

#include "DX12Includes.h"
#include <stdexcept>

class IndexBuffer12
{
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW ibView = {};
public:
	IndexBuffer12() {}
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
			IID_PPV_ARGS(&indexBuffer)
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
			IID_PPV_ARGS(&indexUploadBuffer)
		);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create upload heap");
		}

		// Upload data
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = reinterpret_cast<const BYTE*>(data);
		indexData.RowPitch = ibSize;
		indexData.SlicePitch = ibSize;

		UpdateSubresources(commandList, indexBuffer.Get(), indexUploadBuffer.Get(), 0, 0, 1, &indexData);
		
		CD3DX12_RESOURCE_BARRIER ibBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			indexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER
		);
		commandList->ResourceBarrier(1, &ibBarrier);

		// Fill out IB view
		ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		ibView.SizeInBytes = ibSize;
		ibView.Format = DXGI_FORMAT_R32_UINT;

		return S_OK;
	}


	D3D12_GPU_VIRTUAL_ADDRESS GetIndexBufferVirtualAddress() const
	{
		return indexBuffer->GetGPUVirtualAddress();
	}
};
