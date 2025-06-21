#pragma once
#include "DX12Includes.h"
#include "MeshData.h"
#include <unordered_map>

namespace ECS
{
	class MeshManager
	{
	public:
		MeshManager();
		void CreateMesh(const std::string& name, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const MeshData& data);
		std::shared_ptr<GpuMesh> Get(const std::string& name) const;

	private:
		std::unordered_map<std::string, std::shared_ptr<GpuMesh>> gpuMeshes;

	};
}


