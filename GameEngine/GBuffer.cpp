#include "GBuffer.h"

GBuffer::GBuffer()
{

}

void GBuffer::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* commandAllocator, 
	ID3D12DescriptorHeap* sharedRsvHeap, DescriptorAllocator* descriptorAllocator, uint32_t width, uint32_t height)
{
	m_gBufferTexture.Initialize(device, cmdList, commandAllocator, sharedRsvHeap, descriptorAllocator, width, height, GBUFFER_TEXTURES_NUM);
}

RenderTargetTexture& GBuffer::GetGbufferRenderTargetTexture()
{
	return m_gBufferTexture;
}

void GBuffer::ResetRenderTarget(ID3D12GraphicsCommandList* cmdList)
{
	m_gBufferTexture.Reset(cmdList);
}

void GBuffer::GBufferPass(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle)
{
	m_gBufferTexture.SetRenderTarget(cmdList, dsvHandle);
}

void GBuffer::LightPass()
{

}