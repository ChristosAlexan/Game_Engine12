#pragma once
#include "MeshData.h"

enum SHAPE_TYPE
{
	QUAD = 0,
	CUBE = 1
};

ECS::MeshData GenerateCubeMesh();
ECS::MeshData GenerateQuadMesh();
ECS::MeshData GenerateSphereMesh(uint32_t segments = 16, uint32_t rings = 16);