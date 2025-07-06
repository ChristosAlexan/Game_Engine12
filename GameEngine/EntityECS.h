#pragma once
#include "MeshGenerators.h"
#include "TransformECS.h"
#include "MaterialECS.h"

namespace ECS
{
	struct EntityDesc
	{
		std::string name;
		std::string filePath;
		std::vector<std::string> anim_filePaths;
		MESH_TYPE meshType;
		std::string materialName = "defaultMaterial";
		TransformComponent transform = {};
		MaterialDesc materialDesc = MaterialDesc::Default();
		AnimatorComponent animComponent;
		bool visible = true;
		bool castShadows = true;
		bool hasMaterial = true;
		bool hasAnimation = false;
	};
}
