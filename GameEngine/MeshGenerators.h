#pragma once
#include "MeshData.h"

ECS::MeshData GenerateCubeMesh();
ECS::MeshData GenerateQuadMesh();
ECS::MeshData GenerateSphereMesh(uint32_t segments = 16, uint32_t rings = 16);