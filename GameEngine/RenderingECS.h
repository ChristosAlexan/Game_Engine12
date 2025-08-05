#pragma once
#include <memory>
#include "Model.h"
#include "MaterialECS.h"
#include "MeshData.h"
#include "RayTraceData.h"

namespace ECS
{
    struct RenderComponent 
    {
        ECS::MESH_TYPE meshType;
        std::shared_ptr<Model> model;
        std::shared_ptr<GpuMesh> mesh;
        std::shared_ptr<Material> material;
        std::string name;
        bool hasAnimation = false;
        bool hasTextures = false;
    };
}
