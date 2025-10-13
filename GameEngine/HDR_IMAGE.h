#pragma once
#include <string>
#include "Texture12.h"

class HDR_IMAGE
{
public:
	HDR_IMAGE();
	~HDR_IMAGE();
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* descriptorAllocator,
		const std::string& filepath);
	Texture12* GetHDRtexture() const;
private:
	std::unique_ptr<Texture12> m_texture;
};

