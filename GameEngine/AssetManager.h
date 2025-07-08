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
        std::shared_ptr<GpuMesh> GetOrLoadMesh(EntityDesc& entityDesc, entt::registry* registry, entt::entity& entity, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
        void MapModel(Model& model, EntityDesc& entityDesc);
        std::shared_ptr<Model> GetModel(const std::string& modelName);
    public:
        std::unordered_map<std::string, std::shared_ptr<GpuMesh>> m_meshes;
        std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
    };
}


