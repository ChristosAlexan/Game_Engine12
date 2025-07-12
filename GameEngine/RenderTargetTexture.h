#pragma once
#include "DX12Includes.h"
#include "DescriptorAllocator.h"

class RenderTargetTexture
{
public:
	RenderTargetTexture();
	HRESULT Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* commandAllocator,
		ID3D12DescriptorHeap* sharedRsvHeap, DescriptorAllocator* descriptorAllocator, 
		const uint32_t width, const uint32_t height, std::vector<DXGI_FORMAT>& formats, const uint32_t renderTargets_size);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle(uint32_t index);
	void SetRenderTarget(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle);
	ID3D12Resource* GetRenderTextureSource(uint32_t index);
	void Reset(ID3D12GraphicsCommandList* cmdList);
	void TransitionToRTV(ID3D12GraphicsCommandList* cmdList);
	void TransitionToSRV(ID3D12GraphicsCommandList* cmdList);
	void RenderFullScreenQuad(ID3D12GraphicsCommandList* cmdList,
		ID3D12DescriptorHeap* rtvHeap, ID3D12DescriptorHeap* dsvHeap, UINT& frameIndex, UINT& rtvDescriptorSize,
		ID3D12PipelineState* pipelineState);
private:
	uint32_t m_renderTargets_size = 0; // Total number of render targets to create
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_renderTextures;
	std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> m_rtvHeaps;
	std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> m_rtvHandles;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_cpuHandle;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_gpuHandle;

	ID3D12DescriptorHeap* m_srvHeap = nullptr;
	UINT m_rtvDescriptorSize = 0;
	UINT m_srvDescriptorSize = 0;
};

