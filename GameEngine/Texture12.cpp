#include "Texture12.h"
#include "COMException.h"
#include <DirectXTex.h>
#include "ErrorLogger.h"
#include <iostream>

Texture12::Texture12()
{
}

Texture12::~Texture12()
{
	if (m_resource) 
	{
		m_resource.Reset();
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
		&texDesc, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(&m_resource));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create texture resource!");

	// Upload heap
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, 1);
	m_uploadBuffer.Reset();
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	hr = device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&m_uploadBuffer));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create upload buffer");

	// Copy image data
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = image->pixels;
	subresourceData.RowPitch = image->rowPitch;
	subresourceData.SlicePitch = image->slicePitch;

	UpdateSubresources(cmdList, m_resource.Get(), m_uploadBuffer.Get(), 0, 0, 1, &subresourceData);

	// Transition to shader
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdList->ResourceBarrier(1, &barrier);

	// Create SRV
	DescriptorAllocator::DescriptorHandle handle = descriptorAllocator->Allocate();
	m_cpuHandle = handle.cpuHandle;
	m_gpuHandle = handle.gpuHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_cpuHandle);
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
		&texDesc, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(&m_resource));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create texture resource!");

	// Upload heap
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, 1);
	m_uploadBuffer.Reset();
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	hr = device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&m_uploadBuffer));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create upload buffer");

	// Copy image data
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = image->pixels;
	subresourceData.RowPitch = image->rowPitch;
	subresourceData.SlicePitch = image->slicePitch;

	UpdateSubresources(cmdList, m_resource.Get(), m_uploadBuffer.Get(), 0, 0, 1, &subresourceData);

	// Transition to shader
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdList->ResourceBarrier(1, &barrier);

	// Create SRV
	DescriptorAllocator::DescriptorHandle handle = descriptorAllocator->Allocate();
	m_cpuHandle = handle.cpuHandle;
	m_gpuHandle = handle.gpuHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_cpuHandle);
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
		&texDesc, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(&m_resource));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create texture resource!");

	// Upload heap
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, 1);
	m_uploadBuffer.Reset();
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	hr = device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&m_uploadBuffer));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create upload buffer");

	// Copy image data
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = image->pixels;
	subresourceData.RowPitch = image->rowPitch;
	subresourceData.SlicePitch = image->slicePitch;

	UpdateSubresources(cmdList, m_resource.Get(), m_uploadBuffer.Get(), 0, 0, 1, &subresourceData);

	// Transition to shader
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdList->ResourceBarrier(1, &barrier);

	// Create SRV
	DescriptorAllocator::DescriptorHandle handle = descriptorAllocator->Allocate();
	m_cpuHandle = handle.cpuHandle;
	m_gpuHandle = handle.gpuHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_cpuHandle);
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
	auto barrierToSRV = CD3DX12_RESOURCE_BARRIER::Transition(
		m_resource.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	cmdList->ResourceBarrier(1, &barrierToSRV);
}
void Texture12::TransitionToSRV(ID3D12GraphicsCommandList* cmdList)
{
	auto barrierToSRV = CD3DX12_RESOURCE_BARRIER::Transition(
		m_resource.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	cmdList->ResourceBarrier(1, &barrierToSRV);
}

void Texture12::CreateTextureUAV(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* descriptorAllocator, const UINT width, const UINT height)
{
	// texture descriptor
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
	HRESULT hr = device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE,
		&texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr, IID_PPV_ARGS(&m_resource));
	if (FAILED(hr))
		ErrorLogger::Log(hr, "Failed to create texture resource!");

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = texDesc.Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	// Create UAV
	DescriptorAllocator::DescriptorHandle handle = descriptorAllocator->Allocate();
	m_cpuHandleUAV = handle.cpuHandle;
	m_gpuHandleUAV = handle.gpuHandle;

	device->CreateUnorderedAccessView(m_resource.Get(), nullptr, &uavDesc, m_cpuHandleUAV);

	// Create SRV
	handle = descriptorAllocator->Allocate();
	m_cpuHandle = handle.cpuHandle;
	m_gpuHandle = handle.gpuHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_cpuHandle);
}

void Texture12::Reset(ID3D12GraphicsCommandList* cmdList)
{
	auto barrierToRTV = CD3DX12_RESOURCE_BARRIER::Transition(
		m_resource.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	cmdList->ResourceBarrier(1, &barrierToRTV);
	
}