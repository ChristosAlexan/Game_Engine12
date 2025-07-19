#include "RenderTargetTexture.h"
#include "COMException.h"

RenderTargetTexture::RenderTargetTexture()
{
}

HRESULT RenderTargetTexture::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* commandAllocator, 
	ID3D12DescriptorHeap* sharedRsvHeap, DescriptorAllocator* descriptorAllocator, 
	const uint32_t width, const uint32_t height, std::vector<DXGI_FORMAT>& formats, const uint32_t renderTargets_size)
{
	HRESULT hr;
	m_srvHeap = sharedRsvHeap;
	m_renderTargets_size = renderTargets_size;
	m_rtvHeaps.resize(m_renderTargets_size);
	m_renderTextures.resize(m_renderTargets_size);
	m_rtvHandles.resize(m_renderTargets_size);
	m_cpuHandle.resize(m_renderTargets_size);
	m_gpuHandle.resize(m_renderTargets_size);

	for (uint32_t i = 0; i < m_renderTargets_size; ++i)
	{
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = 0;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = formats[i];
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = formats[i];
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 1.0f;

		// Create Render Target Views
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = m_renderTargets_size;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeaps[i]));
		COM_ERROR_IF_FAILED(hr, "Failed to create RTV descriptor heap");

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		hr = device->CreateCommittedResource(&heapProps,
			D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RENDER_TARGET,
			&clearValue, IID_PPV_ARGS(&m_renderTextures[i]));
		COM_ERROR_IF_FAILED(hr, "Failed to create commited resource!");
		m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // Size of the rtvHeap
		m_rtvHandles[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeaps[i]->GetCPUDescriptorHandleForHeapStart(), i, m_rtvDescriptorSize); // offset per rtv heap to get the correct handle
		device->CreateRenderTargetView(m_renderTextures[i].Get(), nullptr, m_rtvHandles[i]);
	}
		TransitionToSRV(cmdList);

	for (uint32_t i = 0; i < m_renderTargets_size; ++i)
	{

		DescriptorAllocator::DescriptorHandle handle = descriptorAllocator->Allocate();
		m_cpuHandle[i] = handle.cpuHandle;
		m_gpuHandle[i] = handle.gpuHandle;

		// Create SRV
		D3D12_SHADER_RESOURCE_VIEW_DESC srvHeapDesc = {};
		srvHeapDesc.Format = formats[i];
		srvHeapDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvHeapDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvHeapDesc.Texture2D.MipLevels = 1;
		device->CreateShaderResourceView(m_renderTextures[i].Get(), &srvHeapDesc, m_cpuHandle[i]);
	}

	return hr;
}

HRESULT RenderTargetTexture::InitializeCubeMap(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* commandAllocator,
	ID3D12DescriptorHeap* sharedRsvHeap, DescriptorAllocator* descriptorAllocator,
	const uint32_t width, const uint32_t height)
{
	m_width = width;
	m_height = height;

	HRESULT hr;
	m_srvHeap = sharedRsvHeap;
	m_renderTargets_size = 6;
	m_rtvHeaps.resize(1);
	m_renderTextures.resize(1);
	m_rtvHandles.resize(6);
	m_cpuHandle.resize(1);
	m_gpuHandle.resize(1);

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 6; // 6 faces
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	// Create Render Target Views
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 6;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeaps[0]));
	COM_ERROR_IF_FAILED(hr, "Failed to create RTV descriptor heap");

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	hr = device->CreateCommittedResource(&heapProps,
		D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue, IID_PPV_ARGS(&m_renderTextures[0]));
	COM_ERROR_IF_FAILED(hr, "Failed to create commited resource!");


	
	for (UINT face = 0; face < 6; ++face)
	{
		m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // Size of the rtvHeap

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = face;
		rtvDesc.Texture2DArray.ArraySize = 1;
		rtvDesc.Texture2DArray.PlaneSlice = 0;

		m_rtvHandles[face] = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_rtvHeaps[0]->GetCPUDescriptorHandleForHeapStart(),
			face, m_rtvDescriptorSize);

		device->CreateRenderTargetView(m_renderTextures[0].Get(), &rtvDesc, m_rtvHandles[face]);
	}
	
	TransitionToSRV(cmdList);

	DescriptorAllocator::DescriptorHandle handle = descriptorAllocator->Allocate();
	m_cpuHandle[0] = handle.cpuHandle;
	m_gpuHandle[0] = handle.gpuHandle;

	// Create SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srvHeapDesc = {};
	srvHeapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvHeapDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvHeapDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvHeapDesc.TextureCube.MipLevels = desc.MipLevels;
	srvHeapDesc.TextureCube.MostDetailedMip = 0;
	device->CreateShaderResourceView(m_renderTextures[0].Get(), &srvHeapDesc, m_cpuHandle[0]);
	
	return hr;
}

D3D12_GPU_DESCRIPTOR_HANDLE RenderTargetTexture::GetSrvGpuHandle(uint32_t index)
{
	return m_gpuHandle[index];
}

void RenderTargetTexture::SetRenderTarget(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle, float* clearColor)
{
	//const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdList->OMSetRenderTargets(m_rtvHandles.size(), m_rtvHandles.data(), FALSE, &dsvHandle);
	for (uint32_t i = 0; i < m_rtvHandles.size(); ++i)
	{
		cmdList->ClearRenderTargetView(m_rtvHandles[i], clearColor, 0, nullptr);
	}
}

void RenderTargetTexture::SetRenderTargetIndex(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle, UINT index)
{
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	cmdList->OMSetRenderTargets(1, &m_rtvHandles[index], FALSE, &dsvHandle);

	cmdList->ClearRenderTargetView(m_rtvHandles[index], clearColor, 0, nullptr);
}

ID3D12Resource* RenderTargetTexture::GetRenderTextureSource(uint32_t index)
{
	return m_renderTextures[index].Get();
}

void RenderTargetTexture::Reset(ID3D12GraphicsCommandList* cmdList)
{
	for (uint32_t i = 0; i < m_renderTextures.size(); ++i)
	{
		auto barrierToRTV = CD3DX12_RESOURCE_BARRIER::Transition(
			GetRenderTextureSource(i),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		cmdList->ResourceBarrier(1, &barrierToRTV);
	}

}

void RenderTargetTexture::TransitionToRTV(ID3D12GraphicsCommandList* cmdList)
{
	for(uint32_t i = 0; i < m_renderTextures.size(); ++i)
	{
		auto barrierToSRV = CD3DX12_RESOURCE_BARRIER::Transition(
			GetRenderTextureSource(i),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		cmdList->ResourceBarrier(1, &barrierToSRV);
	}
	
}
void RenderTargetTexture::TransitionToSRV(ID3D12GraphicsCommandList* cmdList)
{
	for (uint32_t i = 0; i < m_renderTextures.size(); ++i)
	{
		auto barrierToSRV = CD3DX12_RESOURCE_BARRIER::Transition(
			GetRenderTextureSource(i),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		cmdList->ResourceBarrier(1, &barrierToSRV);
	}
}
