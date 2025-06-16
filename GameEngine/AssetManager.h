#pragma once
#include <memory>
#include "MaterialECS.h"
#include <string>
#include "MeshData.h"
#include <unordered_map>
#include "Texture12.h"
#include "MeshGenerators.h"

namespace ECS
{
    class AssetManager
    {
    public:
        AssetManager();
        std::shared_ptr<Mesh12> GetOrLoadMesh(SHAPE_TYPE shapeType, const std::string& name, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
       
    public:
        std::unordered_map<std::string, std::shared_ptr<Mesh12>> m_meshes;
    };
}


