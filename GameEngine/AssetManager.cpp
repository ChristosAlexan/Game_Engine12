#include "AssetManager.h"
#include "ErrorLogger.h"

namespace ECS
{
	ECS::AssetManager::AssetManager()
	{
	}

	std::shared_ptr<Mesh12> ECS::AssetManager::GetOrLoadMesh(MESH_TYPE shapeType, const std::string& name, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::string* filepath)
	{
		if (m_meshes.contains(name))
			return m_meshes.at(name);

		MeshData data;

		switch (shapeType)
		{
			case QUAD:
				data = GenerateQuadMesh();
				break;
			case CUBE:
				data = GenerateCubeMesh();
				break;
			case STATIC_MESH:
				data = GenerateStaticMesh(*filepath);
				break;

		}
		auto mesh = std::make_shared<Mesh12>();
		mesh->Upload(device, cmdList, data);
		m_meshes.emplace(name, mesh);

		return m_meshes.at(name);
	}
}


