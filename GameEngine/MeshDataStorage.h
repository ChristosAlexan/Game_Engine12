#pragma once
#include "MeshData.h"
#include <unordered_map>

namespace ECS
{
	class MeshDataStorage
	{
	public:
		MeshDataStorage();
		void InitializePrimitives();
		const ECS::MeshData& GetMeshData(const std::string& name) const;

	private:
		std::unordered_map<std::string, ECS::MeshData> primitives;
	};
}

