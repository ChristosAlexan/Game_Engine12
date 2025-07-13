#pragma once
#include "MeshGenerators.h"
#include "TransformECS.h"
#include "MaterialECS.h"
#include "LightECS.h"

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
		LightComponent lightComponent;
		bool visible = true;
		bool castShadows = false;
		bool hasTextures = false;
		bool hasAnimation = false;
	};
}
