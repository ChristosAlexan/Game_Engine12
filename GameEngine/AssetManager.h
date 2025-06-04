#pragma once
#include <memory>
#include "MaterialECS.h"
#include <string>
#include "MeshData.h"
#include <unordered_map>
#include "Texture12.h"

namespace ECS
{
    class AssetManager
    {
    public:
        AssetManager();
        std::shared_ptr<Mesh12> GetOrLoadMesh(const std::string& name, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
        std::shared_ptr<Material> GetOrCreateMaterial(const std::string& name);
       std::shared_ptr<Texture12> LoadTexture(const std::string& file,
           ID3D12Device* device,
           ID3D12GraphicsCommandList* cmdList,
           DescriptorAllocator& srvAllocator);
       
    public:
        std::unordered_map<std::string, std::shared_ptr<Mesh12>> m_meshes;
        std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
        std::unordered_map<std::string, std::shared_ptr<Texture12>> m_textures;

    };
}


