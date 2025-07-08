#include "AssetManager.h"
#include "ErrorLogger.h"

namespace ECS
{
	ECS::AssetManager::AssetManager()
	{
	}

	std::shared_ptr<GpuMesh> ECS::AssetManager::GetOrLoadMesh(EntityDesc& entityDesc, entt::registry* registry, entt::entity& entity, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		if (m_meshes.contains(entityDesc.name))
			return m_meshes.at(entityDesc.name);

		OutputDebugStringA(( "Name = " + entityDesc.name + "\n").c_str());
		MeshData data;
		Model model;

		switch (entityDesc.meshType)
		{
			case QUAD:
				data = GenerateQuadMesh();
				break;
			case CUBE:
				data = GenerateCubeMesh();
				break;
			case STATIC_MESH:
				data = GenerateStaticMesh(model, entityDesc);
				MapModel(model, entityDesc);
				break;
			case SKELETAL_MESH:
				data = GenerateSkeletalMesh(model, entityDesc);
				MapModel(model, entityDesc);
				break;

		}

		auto mesh = std::make_shared<GpuMesh>();
		mesh->cpuMesh = std::move(data);
		mesh->Upload(device, cmdList);
		m_meshes.emplace(entityDesc.name, mesh);

		return m_meshes.at(entityDesc.name);
	}

	void AssetManager::MapModel(Model& model, EntityDesc& entityDesc)
	{
		m_models.emplace(entityDesc.name, std::make_shared<Model>(model));
	}


	std::shared_ptr<Model> AssetManager::GetModel(const std::string& modelName)
	{
		return m_models.at(modelName);
	}
}


