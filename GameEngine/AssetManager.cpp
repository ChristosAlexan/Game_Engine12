#include "AssetManager.h"
#include "MeshGenerators.h"
#include "ErrorLogger.h"

namespace ECS
{
	ECS::AssetManager::AssetManager()
	{
	}

	std::shared_ptr<Mesh12> ECS::AssetManager::GetOrLoadMesh(const std::string& name, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		if (m_meshes.contains(name))
			return m_meshes.at(name);

		MeshData data;
		if (name == "cube")
			data = GenerateCubeMesh();
		else if (name == "quad")
			data = GenerateQuadMesh();

		auto mesh = std::make_shared<Mesh12>();
		mesh->Upload(device, cmdList, data);
		m_meshes.emplace(name, mesh);

		return m_meshes.at(name);
	}
}


