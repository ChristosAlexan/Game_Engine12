#include "AssetManager.h"
#include "RenderingManager.h"
#include "ErrorLogger.h"
#include "BLASBuilder.h"
#include "DX12.h"

namespace ECS
{
	ECS::AssetManager::AssetManager()
	{
	}

	std::shared_ptr<GpuMesh> ECS::AssetManager::GetOrLoadMesh(DX12& dx12, EntityDesc& entityDesc, entt::registry* registry, entt::entity& entity, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		if (m_meshes.contains(entityDesc.name))
			return m_meshes.at(entityDesc.name);

		MeshData data;
		Model model;
		BLASBuilder blas_builder;

		switch (entityDesc.meshType)
		{
			case QUAD:
				data = GenerateQuadMesh(entityDesc);
				break;
			case CUBE:
				data = GenerateCubeMesh(entityDesc);
				break;
			case STATIC_MESH:
				data = GenerateStaticMesh(model, entityDesc);
				MapModel(model, entityDesc);
				break;
			case SKELETAL_MESH:
				data = GenerateSkeletalMesh(model, entityDesc);
				MapModel(model, entityDesc);
				break;
			case LIGHT:
				data = GenerateCubeMesh(entityDesc);
				break;
		}

		auto mesh = std::make_shared<GpuMesh>();
		mesh->cpuMesh = std::move(data);
		mesh->Upload(device, cmdList);

		if (entityDesc.meshType != ECS::MESH_TYPE::LIGHT)
		{
			mesh->blas = std::make_shared<BLAS>(blas_builder.Build(dx12.GetDevice(), dx12.GetCmdList(),
			mesh->vertexBuffer.GetVertexBufferVirtualAddress(), mesh->vertexCount, mesh->vertexBuffer.vbView.StrideInBytes,
			mesh->indexBuffer.GetIndexBufferVirtualAddress(), mesh->indexCount, mesh->indexBuffer.ibView.Format));
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


