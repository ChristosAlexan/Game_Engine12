#pragma once
#include "MaterialECS.h"
#include <optional>

namespace ECS
{
	class MaterialManager
	{
	public:
		MaterialManager(ID3D12Device* device,
			ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator);
		std::shared_ptr<Material> GetOrCreateMaterial(const MaterialDesc& materialDesc);
		std::shared_ptr<Texture12> GetOrLoadTexture(const std::string& file, const std::string& name,
			ID3D12Device* device,
			ID3D12GraphicsCommandList* cmdList,
			DescriptorAllocator* allocator);

		std::shared_ptr<Material> GetMaterial(std::string name) const;

		void Bindtextures(Material* material, ID3D12GraphicsCommandList* cmdList, UINT rootIndex);

		std::optional<MaterialDesc> GetMaterialDescByName(const std::string& name) const;
	private:
		void AddTexture(const MaterialDesc& materialDesc, Material* material, TEXTURE_TYPE textType,
			ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator);
	public:
		std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
		std::unordered_map<std::string, std::shared_ptr<Texture12>> m_textures;
		std::unordered_map<std::string, MaterialDesc> m_materialDescs;

	private:
		ID3D12Device* m_device;
		ID3D12GraphicsCommandList* m_cmdList;
		DescriptorAllocator* m_allocator;
	};
}


