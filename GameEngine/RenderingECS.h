#pragma once
#include <memory>
#include "Model.h"
#include "MaterialECS.h"
#include "MeshData.h"

namespace ECS
{
    struct RenderComponent 
    {
        std::shared_ptr<Model> model;
        std::shared_ptr<GpuMesh> mesh;
        std::shared_ptr<Material> material;
        std::string name;
        std::vector<std::string> anim_names; // Store the names of the attached animations
        DirectX::XMMATRIX bone_transform;
        bool hasAnimation = false;
        int curAnim = 0;
    };
}
