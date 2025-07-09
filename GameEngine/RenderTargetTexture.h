#pragma once
#include "DX12Includes.h"

class RenderTargetTexture
{
public:
	RenderTargetTexture();
	HRESULT Initialize(ID3D12Device* device, uint32_t width, uint32_t height);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle();
	void SetRenderTarget(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle);
	ID3D12Resource* GetResource();
	void Reset(ID3D12GraphicsCommandList* cmdList);
	void Transition(ID3D12GraphicsCommandList* cmdList);
	void RenderFullScreenQuad(ID3D12GraphicsCommandList* cmdList,
		ID3D12DescriptorHeap* rtvHeap, ID3D12DescriptorHeap* dsvHeap, UINT& frameIndex, UINT& rtvDescriptorSize,
		ID3D12PipelineState* pipelineState);
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTexture;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap, m_srvHeap;
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle;
	UINT m_rtvDescriptorSize = 0;
	UINT m_srvDescriptorSize = 0;
};

