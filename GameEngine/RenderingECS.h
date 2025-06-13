#pragma once
#include <memory>
#include "MaterialECS.h"
#include "MeshData.h"

namespace ECS
{
    struct RenderComponent {
        std::shared_ptr<Mesh12> mesh;
        std::shared_ptr<Material> material;
    };
}
