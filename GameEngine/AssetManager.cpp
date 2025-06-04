#include "AssetManager.h"
#include "MeshGenerators.h"

namespace ECS
{
	ECS::AssetManager::AssetManager()
	{
	}

	std::shared_ptr<Mesh12> ECS::AssetManager::GetOrLoadMesh(const std::string& name, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		if (m_meshes.contains(name))
			return m_meshes[name];

		MeshData data;
		if (name == "cube")
			data = GenerateCubeMesh();
		else if (name == "quad")
			data = GenerateQuadMesh();

		auto mesh = std::make_shared<Mesh12>();
		mesh->Upload(device, cmdList, data);
		m_meshes[name] = mesh;

		return mesh;
	}

	std::shared_ptr<Material> AssetManager::GetOrCreateMaterial(const std::string& name)
	{
		return std::shared_ptr<Material>();
	}

	std::shared_ptr<Texture12> AssetManager::LoadTexture(const std::string& file,
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		DescriptorAllocator& srvAllocator)
	{
		return std::shared_ptr<Texture12>();
	}

}


