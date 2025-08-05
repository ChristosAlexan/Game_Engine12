#include "GBuffer.h"
#include "GFX_MACROS.h"

GBuffer::GBuffer()
{

}

void GBuffer::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* commandAllocator, 
	ID3D12DescriptorHeap* sharedRsvHeap, DescriptorAllocator* descriptorAllocator, uint32_t width, uint32_t height)
{
	m_formats.resize(GBUFFER_TEXTURES_NUM);

	m_formats[GBUFFER_RENDER_TARGETS_FORMAT_MAPPINGS::ALBEDO] = FORMAT_ALBEDO;
	m_formats[GBUFFER_RENDER_TARGETS_FORMAT_MAPPINGS::NORMAL] = FORMAT_NORMAL;
	m_formats[GBUFFER_RENDER_TARGETS_FORMAT_MAPPINGS::ROUGH_METAL] = FORMAT_ROUGH_METAL;
	m_formats[GBUFFER_RENDER_TARGETS_FORMAT_MAPPINGS::WORLDPOS_DEPTH] = FORMAT_WORLDPOS_DEPTH;

	m_gBufferTexture.Initialize(device, cmdList, commandAllocator, sharedRsvHeap, descriptorAllocator, width, height, m_formats, GBUFFER_TEXTURES_NUM);
}

RenderTargetTexture& GBuffer::GetGbufferRenderTargetTexture()
{
	return m_gBufferTexture;
}

void GBuffer::ResetRenderTargets(ID3D12GraphicsCommandList* cmdList)
{
	m_gBufferTexture.Reset(cmdList);
}