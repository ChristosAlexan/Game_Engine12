#include "MeshDataStorage.h"

namespace ECS
{
	MeshDataStorage::MeshDataStorage()
	{
	}

	void MeshDataStorage::InitializePrimitives()
	{
	}

	const MeshData& MeshDataStorage::GetMeshData(const std::string& name) const
	{
		return primitives.at(name);
	}
}

