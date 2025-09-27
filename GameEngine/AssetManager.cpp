#include "AssetManager.h"
#include "RenderingManager.h"
#include "ErrorLogger.h"
#include "BLASBuilder.h"
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
		BLASBuilder blas_builder;

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
				break;
			case SKELETAL_MESH:
				cpuMesh = GenerateSkeletalMesh(model, entityDesc, scene);
				MapModel(model, entityDesc);
				break;
			case LIGHT:
				cpuMesh = GenerateCubeMesh(entityDesc);
				break;
		}

		auto mesh = std::make_shared<GpuMesh>(model.GetGpuMesh());
		mesh->cpuMesh = std::make_shared<MeshData>(cpuMesh);
		mesh->Upload(device, cmdList);

		if (entityDesc.meshType != ECS::MESH_TYPE::LIGHT)
		{
			if (entityDesc.meshType == ECS::MESH_TYPE::STATIC_MESH)
			{
				mesh->staticBlas = std::make_shared<BLAS>(blas_builder.Build(scene->GetRenderingManager()->GetDX12().GetDevice(), scene->GetRenderingManager()->GetDX12().GetCmdList(),
					mesh->vertexBuffer.GetVertexBufferVirtualAddress(), mesh->vertexCount, mesh->vertexBuffer.vbView.StrideInBytes,
					mesh->indexBuffer.GetIndexBufferVirtualAddress(), mesh->indexCount, mesh->indexBuffer.ibView.Format,
					D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE));
			}
			else if (entityDesc.meshType == ECS::MESH_TYPE::SKELETAL_MESH)
			{
				mesh->skinnedBlas = std::make_shared<BLAS>(blas_builder.Build(scene->GetRenderingManager()->GetDX12().GetDevice(), scene->GetRenderingManager()->GetDX12().GetCmdList(),
					mesh->vertexBuffer.GetVertexBufferVirtualAddress(), mesh->vertexCount, mesh->vertexBuffer.vbView.StrideInBytes,
					mesh->indexBuffer.GetIndexBufferVirtualAddress(), mesh->indexCount, mesh->indexBuffer.ibView.Format,
					D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE));
			}
		}

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


