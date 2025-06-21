#pragma once
#include "MeshData.h"
#include "Model.h"

enum MESH_TYPE
{
	QUAD = 0,
	CUBE = 1,
	STATIC_MESH = 2,
	SKELETAL_MESH = 3
};

ECS::MeshData GenerateCubeMesh();
ECS::MeshData GenerateQuadMesh();
ECS::MeshData GenerateStaticMesh(const std::string& filepath);