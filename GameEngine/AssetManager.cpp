#include "AssetManager.h"
#include "ErrorLogger.h"

namespace ECS
{
	ECS::AssetManager::AssetManager()
	{
	}

	std::shared_ptr<GpuMesh> ECS::AssetManager::GetOrLoadMesh(EntityDesc& entityDesc, entt::registry* registry, entt::entity& id, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		if (m_meshes.contains(entityDesc.name))
			return m_meshes.at(entityDesc.name);

		MeshData data;

		switch (entityDesc.meshType)
		{
			case QUAD:
				data = GenerateQuadMesh();
				break;
			case CUBE:
				data = GenerateCubeMesh();
				break;
			case STATIC_MESH:
				data = GenerateStaticMesh(entityDesc);
			case SKELETAL_MESH:
				data = GenerateSkeletalMesh(registry, entityDesc);
				break;

		}

		auto mesh = std::make_shared<GpuMesh>();
		mesh->cpuMesh = std::move(data);
		mesh->Upload(device, cmdList);
		m_meshes.emplace(entityDesc.name, mesh);

		return m_meshes.at(entityDesc.name);
	}
}


