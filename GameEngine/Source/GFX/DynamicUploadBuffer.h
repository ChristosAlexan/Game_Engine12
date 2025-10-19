#pragma once
#include "DX12Includes.h"
#include <stdexcept>

class DynamicUploadBuffer
{
public:
    DynamicUploadBuffer(ID3D12Device* device, UINT totalSize)
    {
        totalSize = Align(totalSize, 256);

        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);

        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadBuffer)
        );

        uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&cpuPtr));
        gpuPtr = uploadBuffer->GetGPUVirtualAddress();
        bufferSize = totalSize;
    }

    ~DynamicUploadBuffer()
    {
        if (uploadBuffer) uploadBuffer->Unmap(0, nullptr);
        cpuPtr = nullptr;
    }

    void Reset()
    {
        offset = 0;
    }

    template <typename T>
    D3D12_GPU_VIRTUAL_ADDRESS Allocate(const T& data)
    {
        UINT size = Align(sizeof(T), 256);
        if (offset + size > bufferSize)
            throw std::runtime_error("Upload buffer overflow");

        memcpy(cpuPtr + offset, &data, sizeof(T));
        D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = gpuPtr + offset;
        offset += size;
        return gpuAddress;
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
    UINT8* cpuPtr = nullptr;
    D3D12_GPU_VIRTUAL_ADDRESS gpuPtr = 0;
    UINT offset = 0;
    UINT bufferSize = 0;

    static constexpr UINT Align(UINT size, UINT alignment)
    {
        return (size + (alignment - 1)) & ~(alignment - 1);
    }
};