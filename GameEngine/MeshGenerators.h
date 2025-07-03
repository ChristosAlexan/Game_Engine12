#pragma once
#include "MeshData.h"
#include "Model.h"
#include <entt/entt.hpp>
#include "EntityECS.h"

ECS::MeshData GenerateCubeMesh();
ECS::MeshData GenerateQuadMesh();
ECS::MeshData GenerateStaticMesh(ECS::EntityDesc& entityDesc);
ECS::MeshData GenerateSkeletalMesh(entt::registry* registry, ECS::EntityDesc& entityDesc);