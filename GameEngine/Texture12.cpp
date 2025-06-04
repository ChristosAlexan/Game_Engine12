#include "Texture12.h"
#include "COMException.h"

Texture12::Texture12()
{
}

void Texture12::LoadFromFile(const std::string& filename, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator& srvAllocator)
{
	DirectX::ScratchImage image;
	DirectX::TexMetadata metadata;

	HRESULT hr = DirectX::LoadFromWICFile((const wchar_t*)filename.c_str(),
		DirectX::WIC_FLAGS_DEFAULT_SRGB, &metadata, image);

	COM_ERROR_IF_FAILED(hr, "Failed to load texture");

	const DirectX::Image* img = image.GetImage(0, 0, 0);
}
