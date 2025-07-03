#pragma once
#include <memory>
#include "MaterialECS.h"
#include <string>
#include "MeshData.h"
#include <unordered_map>
#include "Texture12.h"
#include "MeshGenerators.h"
#include <entt/entt.hpp>

namespace ECS
{
    class AssetManager
    {
    public:
        class EntityECS;
        class Scene;

        AssetManager();
        std::shared_ptr<GpuMesh> GetOrLoadMesh(EntityDesc& entityDesc, entt::registry* registry, entt::entity& id, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
       
    public:
        std::unordered_map<std::string, std::shared_ptr<GpuMesh>> m_meshes;
    };
}


