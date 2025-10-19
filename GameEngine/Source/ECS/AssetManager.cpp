#include "AssetManager.h"
#include "RenderingManager.h"
#include "ErrorLogger.h"
#include "DX12.h"
#include "Scene.h"

namespace ECS
{
	ECS::AssetManager::AssetManager()
	{
	}

	std::shared_ptr<GpuMesh> ECS::AssetManager::GetOrLoadMesh(Scene* scene, EntityDesc& entityDesc, entt::registry* registry, entt::entity& entity, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		if (m_meshes.contains(entityDesc.name))
			return m_meshes.at(entityDesc.name);

		MeshData cpuMesh;
		Model model;
		
		switch (entityDesc.meshType)
		{
			case QUAD:
				cpuMesh = GenerateQuadMesh(entityDesc);
				break;
			case CUBE:
				cpuMesh = GenerateCubeMesh(entityDesc);
				break;
			case STATIC_MESH:
				cpuMesh = GenerateStaticMesh(model, entityDesc);
				MapModel(model, entityDesc);

				registry->emplace<Model>(entity, model);
				break;
			case SKELETAL_MESH:
				cpuMesh = GenerateSkeletalMesh(model, entityDesc, scene);
				MapModel(model, entityDesc);

				registry->emplace<Model>(entity, model);
				break;
			case LIGHT:
				cpuMesh = GenerateCubeMesh(entityDesc);
				break;
		}

		auto mesh = std::make_shared<GpuMesh>(model.GetGpuMesh());
		mesh->cpuMesh = std::make_shared<MeshData>(cpuMesh);
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


