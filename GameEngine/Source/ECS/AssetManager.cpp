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

	static std::string MakeMeshKey(const EntityDesc& d)
	{
		switch (d.meshType)
		{
		case QUAD:  return "builtin:quad";
		case CUBE:
		case LIGHT: return "builtin:cube";                         // lights use the same cube
		case STATIC_MESH:
		case SKELETAL_MESH: return d.filePath;                     // same file = same geometry
		}
		return d.name;
	}

	std::shared_ptr<GpuMesh> ECS::AssetManager::GetOrLoadMesh(Scene* scene, EntityDesc& entityDesc, entt::registry* registry, entt::entity& entity, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		const std::string key = MakeMeshKey(entityDesc);
		if (m_meshes.contains(key))
			return m_meshes.at(key);

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

		m_meshes.emplace(key, mesh);

		return m_meshes.at(key);
	}

	void AssetManager::MapModel(Model& model, EntityDesc& entityDesc)
	{
		m_models.emplace(MakeMeshKey(entityDesc), std::make_shared<Model>(model));
	}	


	std::shared_ptr<Model> AssetManager::GetModel(const EntityDesc& d)
	{
		return m_models.at(MakeMeshKey(d));
	}

	void AssetManager::UploadGlobalBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		globalVertexBuffer.Initialize(device, cmdList, globalVertices.data(), globalVertices.size());
		globalIndexBuffer.Initialize(device, cmdList, globalIndices.data(), globalIndices.size());
	}
}


