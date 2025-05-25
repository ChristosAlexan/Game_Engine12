#include "MeshManager.h"

namespace ECS
{
	MeshManager::MeshManager()
	{
	}

	void MeshManager::CreateMesh(const std::string& name, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const MeshData& data)
	{
		std::shared_ptr<Mesh12> gpuMesh = std::make_shared<Mesh12>();
		gpuMesh->Upload(device, cmdList, data);
		gpuMeshes[name] = gpuMesh;
	}

	std::shared_ptr<Mesh12> MeshManager::Get(const std::string& name) const
	{
		return gpuMeshes.at(name);
	}
}

