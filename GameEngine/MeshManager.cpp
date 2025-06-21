#include "MeshManager.h"

namespace ECS
{
	MeshManager::MeshManager()
	{
	}

	void MeshManager::CreateMesh(const std::string& name, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const MeshData& data)
	{
		std::shared_ptr<GpuMesh> gpuMesh = std::make_shared<GpuMesh>();
		gpuMesh->cpuMesh = std::move(data);
		gpuMesh->Upload(device, cmdList);
		gpuMeshes[name] = gpuMesh;
	}

	std::shared_ptr<GpuMesh> MeshManager::Get(const std::string& name) const
	{
		return gpuMeshes.at(name);
	}
}

