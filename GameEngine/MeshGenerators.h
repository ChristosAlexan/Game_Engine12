#pragma once
#include "MeshData.h"

enum MESH_TYPE
{
	QUAD = 0,
	CUBE = 1,
	MESH = 2,
	SKELETAL_MESH = 3
};

ECS::MeshData GenerateCubeMesh();
ECS::MeshData GenerateQuadMesh();
ECS::MeshData GenerateSphereMesh(uint32_t segments = 16, uint32_t rings = 16);