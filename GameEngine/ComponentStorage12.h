#pragma once
#include <unordered_map>
#include "ECSHeader.h"
#include "TransformECS.h"

namespace ECS
{
    struct ComponentStorage12 {
        std::unordered_map<EntityID, TransformComponent> transforms;
    };

}