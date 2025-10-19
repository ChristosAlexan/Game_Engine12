#pragma once

#include "DX12Includes.h"
#include <vector>
#include <stdexcept>
#include "ResourceWrapper.h"

template<class T>
class VertexBuffer12
{
private:
    ResourceWrapper* m_vertexBuffer = nullptr;
    ResourceWrapper* m_vertexUploadBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbView = {};
public:
	VertexBuffer12()
	{
        m_vertexBuffer = new ResourceWrapper(D3D12_RESOURCE_STATE_COPY_DEST);
        m_vertexUploadBuffer = new ResourceWrapper(D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	HRESULT Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const T* data, UINT vertexCount)
	{
		HRESULT hr;
        UINT vbSize = sizeof(T) * vertexCount;

        // Create default heap (GPU memory)
        CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vbSize);

        device->CreateCommittedResource(
            &defaultHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, // start in copy dest state
            nullptr,
            IID_PPV_ARGS(m_vertexBuffer->ReleaseAndGetAddressOf())
        );

        // Create upload heap
        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
        device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_vertexUploadBuffer->ReleaseAndGetAddressOf())
        );

        // Copy data to upload heap
        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = reinterpret_cast<const BYTE*>(data);
        vertexData.RowPitch = vbSize;
        vertexData.SlicePitch = vbSize;

        UpdateSubresources(commandList, m_vertexBuffer->GetResource(), m_vertexUploadBuffer->GetResource(), 0, 0, 1, &vertexData);

        // Transition to vertex buffer state
        m_vertexBuffer->TransitionState(commandList, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        // Set view
        m_vbView.BufferLocation = m_vertexBuffer->GetResource()->GetGPUVirtualAddress();
        m_vbView.StrideInBytes = sizeof(T);
        m_vbView.SizeInBytes = vbSize;

		return S_OK;
	}

    D3D12_GPU_VIRTUAL_ADDRESS GetVertexBufferVirtualAddress() const
    {
        return m_vertexBuffer->GetResource()->GetGPUVirtualAddress();
    }

    ResourceWrapper* GetResource() const
    {
        return m_vertexBuffer;
    }

    D3D12_VERTEX_BUFFER_VIEW GetBufferView() const
    {
        return m_vbView;
    }

    const D3D12_VERTEX_BUFFER_VIEW* GetBufferViewPtr() const
    {
        return &m_vbView;
    }
};