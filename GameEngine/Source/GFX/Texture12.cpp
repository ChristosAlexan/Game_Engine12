#include "Texture12.h"
#include "COMException.h"
#include <DirectXTex.h>
#include "ErrorLogger.h"
#include <iostream>

Texture12::Texture12()
{
	m_resource = std::make_unique<ResourceWrapper>(D3D12_RESOURCE_STATE_COPY_DEST);
	m_uploadBuffer = std::make_unique<ResourceWrapper>(D3D12_RESOURCE_STATE_GENERIC_READ);
}

Texture12::~Texture12()
{
	if (m_resource->GetComPtrResource())
	{
		m_resource->GetComPtrResource().Reset();
	}
}

void Texture12::LoadFromFileWIC(const std::string& filename, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* descriptorAllocator)
{
	m_debugName = filename;

	DirectX::ScratchImage scratchImage;
	DirectX::TexMetadata metadata;

	std::wstring widePath(filename.begin(), filename.end());

	HRESULT hr = DirectX::LoadFromWICFile(widePath.c_str(),
		DirectX::WIC_FLAGS_FORCE_RGB, &metadata, scratchImage);

	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to load texture!");


	const DirectX::Image* image = scratchImage.GetImage(0, 0, 0);

	// texture descriptor
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = static_cast<UINT>(metadata.width);
	texDesc.Height = static_cast<UINT>(metadata.height);
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
	hr = device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE,
		&texDesc, m_resource->GetCurrentState(),
		nullptr, IID_PPV_ARGS(m_resource->ReleaseAndGetAddressOf()));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create texture resource!");

	// Upload heap
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource->GetResource(), 0, 1);
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	hr = device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, m_uploadBuffer->GetCurrentState(),
		nullptr, IID_PPV_ARGS(m_uploadBuffer->ReleaseAndGetAddressOf()));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create upload buffer");

	// Copy image data
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = image->pixels;
	subresourceData.RowPitch = image->rowPitch;
	subresourceData.SlicePitch = image->slicePitch;

	UpdateSubresources(cmdList, m_resource->GetResource(), m_uploadBuffer->GetResource(), 0, 0, 1, &subresourceData);

	// Transition to pixel shader
	m_resource->TransitionState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// Create SRV
	DescriptorAllocator::DescriptorHandle handle = descriptorAllocator->Allocate();
	m_cpuHandle = handle.cpuHandle;
	m_gpuHandle = handle.gpuHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(m_resource->GetResource(), &srvDesc, m_cpuHandle);
}

void Texture12::LoadFromFileDDS(const std::string& filename, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* descriptorAllocator)
{
	m_debugName = filename;

	DirectX::ScratchImage scratchImage;
	DirectX::TexMetadata metadata;

	std::wstring widePath(filename.begin(), filename.end());

	HRESULT hr = DirectX::LoadFromDDSFile(widePath.c_str(),
		DirectX::DDS_FLAGS_FORCE_RGB, &metadata, scratchImage);

	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to load texture!");


	const DirectX::Image* image = scratchImage.GetImage(0, 0, 0);

	// texture descriptor
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = static_cast<UINT>(metadata.width);
	texDesc.Height = static_cast<UINT>(metadata.height);
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = metadata.format;
	texDesc.SampleDesc.Count = 1;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
	hr = device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE,
		&texDesc, m_resource->GetCurrentState(),
		nullptr, IID_PPV_ARGS(m_resource->ReleaseAndGetAddressOf()));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create texture resource!");

	// Upload heap
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource->GetResource(), 0, 1);
	//m_uploadBuffer->GetResource().Reset();
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	hr = device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, m_uploadBuffer->GetCurrentState(),
		nullptr, IID_PPV_ARGS(m_uploadBuffer->ReleaseAndGetAddressOf()));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create upload buffer");

	// Copy image data
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = image->pixels;
	subresourceData.RowPitch = image->rowPitch;
	subresourceData.SlicePitch = image->slicePitch;

	UpdateSubresources(cmdList, m_resource->GetResource(), m_uploadBuffer->GetResource(), 0, 0, 1, &subresourceData);

	// Transition to pixel shader
	m_resource->TransitionState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// Create SRV
	DescriptorAllocator::DescriptorHandle handle = descriptorAllocator->Allocate();
	m_cpuHandle = handle.cpuHandle;
	m_gpuHandle = handle.gpuHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(m_resource->GetResource(), &srvDesc, m_cpuHandle);
}

void Texture12::LoadFromFileHDR(const std::string& filename, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* descriptorAllocator)
{
	m_debugName = filename;

	DirectX::ScratchImage scratchImage;
	DirectX::TexMetadata metadata;

	std::wstring widePath(filename.begin(), filename.end());

	HRESULT hr = DirectX::LoadFromHDRFile(widePath.c_str(), &metadata, scratchImage);

	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to load texture!");

	m_width = metadata.width;
	m_height = metadata.height;

	const DirectX::Image* image = scratchImage.GetImage(0, 0, 0);

	// texture descriptor
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = static_cast<UINT>(metadata.width);
	texDesc.Height = static_cast<UINT>(metadata.height);
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
	hr = device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE,
		&texDesc, m_resource->GetCurrentState(),
		nullptr, IID_PPV_ARGS(m_resource->ReleaseAndGetAddressOf()));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create texture resource!");

	// Upload heap
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource->GetResource(), 0, 1);
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	hr = device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, m_uploadBuffer->GetCurrentState(),
		nullptr, IID_PPV_ARGS(m_uploadBuffer->ReleaseAndGetAddressOf()));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create upload buffer");

	// Copy image data
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = image->pixels;
	subresourceData.RowPitch = image->rowPitch;
	subresourceData.SlicePitch = image->slicePitch;

	UpdateSubresources(cmdList, m_resource->GetResource(), m_uploadBuffer->GetResource(), 0, 0, 1, &subresourceData);

	// Transition to pixel shader
	m_resource->TransitionState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// Create SRV
	DescriptorAllocator::DescriptorHandle handle = descriptorAllocator->Allocate();
	m_cpuHandle = handle.cpuHandle;
	m_gpuHandle = handle.gpuHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(m_resource->GetResource(), &srvDesc, m_cpuHandle);
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture12::GetGPUHandle() const
{
	return m_gpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture12::GetGPUHandleUAV() const
{
	return m_gpuHandleUAV;
}

void Texture12::TransitionToRTV(ID3D12GraphicsCommandList* cmdList)
{
	m_resource->TransitionState(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
void Texture12::TransitionToSRV(ID3D12GraphicsCommandList* cmdList)
{
	m_resource->TransitionState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Texture12::CreateTextureUAV(ID3D12Device* device, DescriptorAllocator* descriptorAllocator, const TextureDesc& textureDesc)
{
	// Resource
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = textureDesc.width;
	texDesc.Height = textureDesc.height;
	texDesc.DepthOrArraySize = static_cast<UINT16>(textureDesc.slices);
	texDesc.MipLevels = 1;
	texDesc.Format = textureDesc.format;
	texDesc.SampleDesc = { 1, 0 };
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	m_resource->SetCurrentState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
	HRESULT hr = device->CreateCommittedResource(
		&defaultHeap, D3D12_HEAP_FLAG_NONE,
		&texDesc, m_resource->GetCurrentState(),
		nullptr, IID_PPV_ARGS(m_resource->ReleaseAndGetAddressOf()));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create texture resource!");

	// UAV
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = texDesc.Format;

	if (textureDesc.viewDimension == D3D12_UAV_DIMENSION_TEXTURE2DARRAY || textureDesc.slices > 1) 
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = textureDesc.slices;
		uavDesc.Texture2DArray.PlaneSlice = 0;
	}
	else 
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Texture2D.PlaneSlice = 0;
	}

	// Create UAV
	{
		auto handle = descriptorAllocator->Allocate();
		m_cpuHandleUAV = handle.cpuHandle;
		m_gpuHandleUAV = handle.gpuHandle;
		device->CreateUnorderedAccessView(m_resource->GetResource(), nullptr, &uavDesc, m_cpuHandleUAV);
	}

	// SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.format;

	if (uavDesc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE2DARRAY) 
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = textureDesc.slices;
		srvDesc.Texture2DArray.PlaneSlice = 0;
		srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
	}
	else 
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	}

	// Create SRV
	{
		auto handle = descriptorAllocator->Allocate();
		m_cpuHandle = handle.cpuHandle;
		m_gpuHandle = handle.gpuHandle;
		device->CreateShaderResourceView(m_resource->GetResource(), &srvDesc, m_cpuHandle);
	}
}

void Texture12::Reset(ID3D12GraphicsCommandList* cmdList)
{
	m_resource->TransitionState(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

ResourceWrapper* Texture12::GetResource() const
{
	return m_resource.get();
}
