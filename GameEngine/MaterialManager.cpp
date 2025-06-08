#include "MaterialManager.h"
#include "ErrorLogger.h"

namespace ECS
{
	MaterialManager::MaterialManager()
	{
	}
	std::shared_ptr<Material> MaterialManager::GetOrCreateMaterial(const MaterialDesc& materialDesc, ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator)
	{
		if (m_materials.contains(materialDesc.name))
			return m_materials.at(materialDesc.name);

		
		auto material = std::make_shared<Material>();

		AddTexture(materialDesc, material.get(), TEXTURE_TYPE::DIFFUSE,  device, cmdList, allocator);
		AddTexture(materialDesc, material.get(), TEXTURE_TYPE::NORMAL, device, cmdList, allocator);
		AddTexture(materialDesc, material.get(), TEXTURE_TYPE::ROUGHNESS, device, cmdList, allocator);
		AddTexture(materialDesc, material.get(), TEXTURE_TYPE::METALLNESS, device, cmdList, allocator);

		OutputDebugStringA(("material->textures.size = " + std::to_string(material->textures.size()) + "\n").c_str());
		material->baseColor = materialDesc.baseColor;
		material->roughness = materialDesc.roughness;
		material->metalness = materialDesc.metalness;
		material->useAlbedoMap = true;
		material->useNormalMap = false;
		material->useMetalnessMap = false;
		material->useRoughnessMap = false;

		m_materials.emplace(materialDesc.name, material);

		return m_materials.at(materialDesc.name);
	}

	std::shared_ptr<Texture12> MaterialManager::GetOrLoadTexture(const std::string& file, const std::string& name,
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		DescriptorAllocator* allocator)
	{
		if (m_textures.contains(name))
			return m_textures.at(name);

		auto texture = std::make_shared<Texture12>();
		texture->LoadFromFile(file, device, cmdList, allocator);
		m_textures.emplace(name, texture);

		return m_textures.at(name);
	}
	void MaterialManager::Bindtextures(Material* material, ID3D12GraphicsCommandList* cmdList, UINT rootIndex)
	{
		cmdList->SetGraphicsRootDescriptorTable(rootIndex, material->textures[0]->GetGPUHandle());
	}
	void MaterialManager::AddTexture(const MaterialDesc& materialDesc, Material* material, TEXTURE_TYPE textType,
		ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator* allocator)
	{
		switch (textType)
		{
			case DIFFUSE:
			{
				GetOrLoadTexture(materialDesc.albedoTexturePath, materialDesc.albedoTextureName, device, cmdList, allocator);
				if (m_textures.contains(materialDesc.albedoTextureName))
					material->albedoTexture = m_textures.at(materialDesc.albedoTextureName);
				material->textures.push_back(material->albedoTexture);
				break;
			}
			case NORMAL:
			{
				GetOrLoadTexture(materialDesc.normalTexturePath, materialDesc.normalTextureName, device, cmdList, allocator);
				if (m_textures.contains(materialDesc.normalTextureName))
					material->normalTexture = m_textures.at(materialDesc.normalTextureName);
				material->textures.push_back(material->normalTexture);
				break;
			}
			case ROUGHNESS:
			{
				GetOrLoadTexture(materialDesc.roughnessTexturePath, materialDesc.roughnessTextureName, device, cmdList, allocator);
				if (m_textures.contains(materialDesc.roughnessTextureName))
					material->roughnessTexture = m_textures.at(materialDesc.roughnessTextureName);
				material->textures.push_back(material->roughnessTexture);
				break;
			}
			case METALLNESS:
			{
				GetOrLoadTexture(materialDesc.metalnessTexturePath, materialDesc.metalnessTextureName, device, cmdList, allocator);
				if (m_textures.contains(materialDesc.metalnessTextureName))
					material->metalnessTexture = m_textures.at(materialDesc.metalnessTextureName);
				material->textures.push_back(material->metalnessTexture);
				break;
			}
			default:
				ErrorLogger::Log("Couldn't find texture!");
				return;
		}
		
	}
}