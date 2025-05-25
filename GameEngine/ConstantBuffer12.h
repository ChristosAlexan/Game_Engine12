#pragma once

#include "DX12Includes.h"

template <class T>
class ConstantBuffer12
{
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
	UINT8* mappedData = nullptr;
	T data;
public:

	ConstantBuffer12() {}
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
	{
		return buffer->GetGPUVirtualAddress();
	}

	HRESULT Initialize(ID3D12Device* device)
	{
		// Must be aligned to 256 bytes in DX12
		static_assert(sizeof(T) <= D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
			"CB size exceeds 256 bytes alignment limit (use struct padding or split)");

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = Align(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&buffer)
		);
		if (FAILED(hr)) return hr;

		// Persistent map
		CD3DX12_RANGE readRange(0, 0); // We won't read from it on the CPU
		hr = buffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData));
		return hr;
	}

	void Update()
	{
		memcpy(mappedData, &data, sizeof(T));
	}

private:
	static constexpr UINT Align(UINT size, UINT alignment)
	{
		return (size + (alignment - 1)) & ~(alignment - 1);
	}
};