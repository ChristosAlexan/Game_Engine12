#pragma once
#include <memory>
#include "Model.h"
#include "MaterialECS.h"
#include "MeshData.h"
#include "RayTraceData.h"

namespace ECS
{
    struct SkinningOutputData
    {
        D3D12_CPU_DESCRIPTOR_HANDLE skinningCpuUavHandleFinalTransform{};
        D3D12_GPU_DESCRIPTOR_HANDLE skinningGpuUavHandleFinalTransform{};
        D3D12_CPU_DESCRIPTOR_HANDLE skinningCpuSrvHandleFinalTransform{};
        D3D12_GPU_DESCRIPTOR_HANDLE skinningGpuSrvHandleFinalTransform{};
        StructuredBuffer<GPUSkinningBufferVertexDataOutput> skinningVertexBufferFinalTransform;
    };

    struct RenderComponent 
    {
        ECS::MESH_TYPE meshType;
        std::shared_ptr<Model> model;
        std::shared_ptr<GpuMesh> mesh;
        std::shared_ptr<Material> material;
        std::string name;
        bool hasAnimation = false;
        bool hasTextures = false;
        SkinningOutputData skinningOutData; // Data per render component for compute skinning
        std::shared_ptr<BLAS> blas; // BLAS per render component
    };
}
