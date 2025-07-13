#pragma once
#include "MeshData.h"
#include "Model.h"
#include <entt/entt.hpp>
#include "EntityECS.h"

ECS::MeshData GenerateCubeMesh(ECS::EntityDesc& entityDesc);
ECS::MeshData GenerateQuadMesh(ECS::EntityDesc& entityDesc);
ECS::MeshData GenerateStaticMesh(Model& model, ECS::EntityDesc& entityDesc);
ECS::MeshData GenerateSkeletalMesh(Model& model, ECS::EntityDesc& entityDesc);