#pragma once
#include "MeshGenerators.h"
#include "TransformECS.h"
#include "MaterialECS.h"

namespace ECS
{
	struct EntityDesc
	{
		std::string name;
		MESH_TYPE meshType = MESH_TYPE::CUBE;
		std::string materialName = "defaultMaterial";
		TransformComponent transform = {};
		MaterialDesc materialDesc = MaterialDesc::Default();
		bool visible = true;
		bool castShadows = true;
		// Optional: Add light, physics, audio etc. later
	};
}
