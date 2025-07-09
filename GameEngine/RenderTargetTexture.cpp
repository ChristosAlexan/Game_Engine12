#include "RenderTargetTexture.h"
#include "COMException.h"

RenderTargetTexture::RenderTargetTexture()
{
}

HRESULT RenderTargetTexture::Initialize(ID3D12Device* device, uint32_t width, uint32_t height)
{
	HRESULT hr;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	hr = device->CreateCommittedResource(&heapProps,
		D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue, IID_PPV_ARGS(&m_renderTexture));

	COM_ERROR_IF_FAILED(hr, "Failed to create commited resource!");

	// Create Render Target View
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
	COM_ERROR_IF_FAILED(hr, "Failed to create RTV descriptor heap");

	m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	device->CreateRenderTargetView(m_renderTexture.Get(), nullptr, rtvHandle);

	// Create Shader Resource View
	m_srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvHeap));
	COM_ERROR_IF_FAILED(hr, "Failed to create srv descriptor heap");
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvHeapDesc = {};
	srvHeapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvHeapDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvHeapDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvHeapDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(m_renderTexture.Get(), &srvHeapDesc, srvHandle);
	srvGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_srvDescriptorSize);

	return hr;
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetTexture::GetRtvHandle()
{
	return m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE RenderTargetTexture::GetSrvGpuHandle()
{
	return srvGpuHandle;
}

void RenderTargetTexture::SetRenderTarget(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle)
{
	Transition(cmdList);

	const float clearColor[] = { 0.1f, 0.1f, 0.4f, 1.0f };

	auto rtvHandle = GetRtvHandle();
	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

ID3D12Resource* RenderTargetTexture::GetResource()
{
	return m_renderTexture.Get();
}

void RenderTargetTexture::Reset(ID3D12GraphicsCommandList* cmdList)
{
	auto barrierToRTV = CD3DX12_RESOURCE_BARRIER::Transition(
		GetResource(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	cmdList->ResourceBarrier(1, &barrierToRTV);
}

void RenderTargetTexture::Transition(ID3D12GraphicsCommandList* cmdList)
{
	auto barrierToSRV = CD3DX12_RESOURCE_BARRIER::Transition(
		GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	cmdList->ResourceBarrier(1, &barrierToSRV);
}

void RenderTargetTexture::RenderFullScreenQuad(ID3D12GraphicsCommandList* cmdList, 
	ID3D12DescriptorHeap* rtvHeap, ID3D12DescriptorHeap* dsvHeap, UINT& frameIndex, UINT& rtvDescriptorSize,
	ID3D12PipelineState* pipelineState)
{
	// Set render target
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	// Set depth-stencil
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	cmdList->SetGraphicsRootDescriptorTable(2, GetSrvGpuHandle());
	cmdList->SetPipelineState(pipelineState);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 0, nullptr);

	cmdList->DrawInstanced(3, 1, 0, 0);
}
