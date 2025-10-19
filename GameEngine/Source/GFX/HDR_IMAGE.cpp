#include "HDR_IMAGE.h"
#include <stdexcept>

HDR_IMAGE::HDR_IMAGE()
{
	m_texture = std::make_unique<Texture12>();
}

HDR_IMAGE::~HDR_IMAGE()
{
}

void HDR_IMAGE::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* descriptorAllocator,
	const std::string& filepath)
{
	m_texture->LoadFromFileHDR(filepath, device, cmdList, descriptorAllocator);
}

Texture12* HDR_IMAGE::GetHDRtexture() const
{
	return m_texture.get();
}
