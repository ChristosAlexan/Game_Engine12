#pragma once

#include "DX12Includes.h"
#include <vector>
#include <stdexcept>

template<class T>
class VertexBuffer12
{
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexUploadBuffer;

	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexUploadBuffer;

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_INDEX_BUFFER_VIEW ibView = {};

public:
	VertexBuffer12()
	{

	}

	HRESULT Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const T* data, UINT vertexCount)
	{
		HRESULT hr;
        UINT vbSize = sizeof(T) * vertexCount;

        // 1. Create default heap (GPU memory)
        CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vbSize);

        device->CreateCommittedResource(
            &defaultHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, // start in copy dest state
            nullptr,
            IID_PPV_ARGS(&vertexBuffer)
        );

        // 2. Create upload heap
        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
        device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexUploadBuffer)
        );

        // 3. Copy data to upload heap
        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = reinterpret_cast<const BYTE*>(data);
        vertexData.RowPitch = vbSize;
        vertexData.SlicePitch = vbSize;

        UpdateSubresources(commandList, vertexBuffer.Get(), vertexUploadBuffer.Get(), 0, 0, 1, &vertexData);

        // 4. Transition to vertex buffer state
        CD3DX12_RESOURCE_BARRIER vbBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            vertexBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
        );
        commandList->ResourceBarrier(1, &vbBarrier);

        // 5. Set view
        vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vbView.StrideInBytes = sizeof(T);
        vbView.SizeInBytes = vbSize;

		return S_OK;
	}
};