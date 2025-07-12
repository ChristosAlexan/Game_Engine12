#pragma once
#include "RenderTargetTexture.h"

class GBuffer
{
public:

	GBuffer();
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* commandAllocator,
		ID3D12DescriptorHeap* sharedRsvHeap, DescriptorAllocator* descriptorAllocator, uint32_t width, uint32_t height);
	RenderTargetTexture& GetGbufferRenderTargetTexture();
	void ResetRenderTarget(ID3D12GraphicsCommandList* cmdList);
	void GBufferPass(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle);
	void LightPass();

private:
	RenderTargetTexture m_gBufferTexture;
	std::vector<DXGI_FORMAT> m_formats; // Formats of each render target
};

