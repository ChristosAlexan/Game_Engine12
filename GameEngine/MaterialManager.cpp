#include "MaterialManager.h"
#include "ErrorLogger.h"
#include <filesystem>

namespace ECS
{
	MaterialManager::MaterialManager(ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator)
		:m_device(device), m_cmdList(cmdList), m_allocator(allocator)
	{
	}
	std::shared_ptr<Material> MaterialManager::GetOrCreateMaterial(const MaterialDesc& materialDesc)
	{
		if (!m_materialDescs.contains(materialDesc.name))
			m_materialDescs.emplace(materialDesc.name, materialDesc);

		if (m_materials.contains(materialDesc.name))
			return m_materials.at(materialDesc.name);

		
		auto material = std::make_shared<Material>();

		if(materialDesc.useAlbedoMap)
			AddTexture(materialDesc, material.get(), TEXTURE_TYPE::DIFFUSE, m_device, m_cmdList, m_allocator, materialDesc.tex_format);
		if (materialDesc.useNormalMap)
			AddTexture(materialDesc, material.get(), TEXTURE_TYPE::NORMAL, m_device, m_cmdList, m_allocator, materialDesc.tex_format);
		if (materialDesc.useMetalRoughnessMap)
			AddTexture(materialDesc, material.get(), TEXTURE_TYPE::METAL_ROUGHNESS, m_device, m_cmdList, m_allocator, materialDesc.tex_format);


		material->name = materialDesc.name;
		material->baseColor = materialDesc.baseColor;
		material->roughness = materialDesc.roughness;
		material->metalness = materialDesc.metalness;
		material->useAlbedoMap = materialDesc.useAlbedoMap;
		material->useNormalMap = materialDesc.useNormalMap;
		material->useMetalRoughnessMap = materialDesc.useMetalRoughnessMap;

		m_materials.emplace(materialDesc.name, material);

		return m_materials.at(materialDesc.name);
	}

	std::shared_ptr<Texture12> MaterialManager::GetOrLoadTexture(const std::string& file, const std::string& name,
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		DescriptorAllocator* allocator, Texture12::TEXTURE_FORMAT tex_format)
	{
		if (m_textures.contains(name))
			return m_textures.at(name);

		auto texture = std::make_shared<Texture12>();

		std::filesystem::path ext = std::filesystem::path(file).extension();
		if (ext == ".dds")
			tex_format = Texture12::TEXTURE_FORMAT::DDS_FILE;
		else
			tex_format = Texture12::TEXTURE_FORMAT::WIC_FILE;

		if(tex_format == Texture12::TEXTURE_FORMAT::DDS_FILE)
			texture->LoadFromFileDDS(file, device, cmdList, allocator);
		else if(tex_format == Texture12::TEXTURE_FORMAT::WIC_FILE)
			texture->LoadFromFileWIC(file, device, cmdList, allocator);
		
		m_textures.emplace(name, texture);

		return m_textures.at(name);
	}

	std::shared_ptr<Material> MaterialManager::GetMaterial(std::string name) const
	{
		if (m_materials.contains(name))
			return m_materials.at(name);
		else
		{
			ErrorLogger::Log("GetMaterial returned null for: " + name);
		}
	}

	void MaterialManager::Bindtextures(Material* material, ID3D12GraphicsCommandList* cmdList, UINT rootIndex)
	{
		cmdList->SetGraphicsRootDescriptorTable(rootIndex, material->textures[0]->GetGPUHandle());
	}

	std::optional<MaterialDesc> MaterialManager::GetMaterialDescByName(const std::string& name) const
	{
		auto it = m_materialDescs.find(name);
		if (it != m_materialDescs.end())
			return it->second;

		return std::nullopt;
	}

	void MaterialManager::AddTexture(const MaterialDesc& materialDesc, Material* material, TEXTURE_TYPE textType,
		ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator, Texture12::TEXTURE_FORMAT tex_format)
	{
		switch (textType)
		{
			case DIFFUSE:
			{
				GetOrLoadTexture(materialDesc.albedoTexturePath, materialDesc.albedoTextureName, device, cmdList, allocator, tex_format);
				if (m_textures.contains(materialDesc.albedoTextureName))
					material->albedoTexture = m_textures.at(materialDesc.albedoTextureName);
				material->textures.push_back(material->albedoTexture);
				break;
			}
			case NORMAL:
			{
				GetOrLoadTexture(materialDesc.normalTexturePath, materialDesc.normalTextureName, device, cmdList, allocator, tex_format);
				if (m_textures.contains(materialDesc.normalTextureName))
					material->normalTexture = m_textures.at(materialDesc.normalTextureName);
				material->textures.push_back(material->normalTexture);
				break;
			}
			case METAL_ROUGHNESS:
			{
				GetOrLoadTexture(materialDesc.metalRoughnessTexturePath, materialDesc.metalRoughnessTextureName, device, cmdList, allocator, tex_format);
				if (m_textures.contains(materialDesc.metalRoughnessTextureName))
					material->metalRoughnessTexture = m_textures.at(materialDesc.metalRoughnessTextureName);
				material->textures.push_back(material->metalRoughnessTexture);
				break;
			}
			default:
				ErrorLogger::Log("Couldn't find texture!");
				return;
		}
		
	}
}