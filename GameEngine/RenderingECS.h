#pragma once
#include <memory>
#include "MaterialECS.h"
#include "MeshData.h"

namespace ECS
{
    struct RenderComponent {
        std::shared_ptr<GpuMesh> mesh;
        std::shared_ptr<Material> material;
        std::string name;
    };
}
