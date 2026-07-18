#pragma once
#include "DX12Includes.h"
#include "VertexBuffer12.h"
#include "IndexBuffer12.h"
#include <vector>
#include "Vertex.h"
#include "MaterialECS.h"
#include "RayTraceData.h"
#include "StructuredBuffer.h"
#include <entt.hpp>

namespace ECS
{
    struct GPUSkinningBufferVertexData
    {
        DirectX::XMFLOAT4 position;
        DirectX::XMFLOAT4 boneWeights;
        uint32_t boneIndices[4] = { 0,0,0,0 };
        DirectX::XMFLOAT4 normal;
        DirectX::XMFLOAT4 tangent;
        DirectX::XMFLOAT4 binormal;
    };

    struct GPUSkinningBufferVertexDataOutput
    {
        DirectX::XMFLOAT3 position;
        float padding;
        DirectX::XMFLOAT3 normal;
        float padding1;
        DirectX::XMFLOAT3 tangent;
        float padding2;
        DirectX::XMFLOAT3 binormal;
        float padding3;
    };

    enum MESH_TYPE
    {
        QUAD = 0,
        CUBE = 1,
        STATIC_MESH = 2,
        SKELETAL_MESH = 3,
        LIGHT = 4
    };

    struct MeshData 
    {
        D3D12_CPU_DESCRIPTOR_HANDLE skinningCpuHandleIn{};
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        MESH_TYPE mesh_type;
        StructuredBuffer<GPUSkinningBufferVertexData> skinningVertexBuffer;
    };

    struct InstanceData
    {
        DirectX::XMFLOAT4X4 worldMatrix;
        DirectX::XMFLOAT4 baseColor;
        float metalness;
        float roughness;
        uint32_t hasTextures;
        uint32_t useAlbedo;
        uint32_t useNormals;
        uint32_t useRoughnessMetal;
        uint32_t materialIndex; // For bindless textures
        uint32_t padding[3];
    };


    struct GpuMesh 
    {
        D3D12_GPU_DESCRIPTOR_HANDLE skinningGpuHandleIn{};
     
        VertexBuffer12<Vertex> vertexBuffer;
        IndexBuffer12 indexBuffer;
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
		std::shared_ptr<ECS::BLAS> blas;
        std::shared_ptr<MeshData> cpuMesh;

        void Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) 
        {
            vertexBuffer.Initialize(device, cmdList, cpuMesh->vertices.data(), cpuMesh->vertices.size());
            vertexCount = static_cast<uint32_t>(cpuMesh->vertices.size());

            if(cpuMesh->indices.size() > 0)
            {
                indexBuffer.Initialize(device, cmdList, cpuMesh->indices.data(), (uint32_t)cpuMesh->indices.size());
                indexCount = static_cast<uint32_t>(cpuMesh->indices.size());
            }
       
        }

        void Draw(ID3D12GraphicsCommandList* cmdList)
        {
            cmdList->IASetVertexBuffers(0, 1, vertexBuffer.GetBufferViewPtr());
            cmdList->DrawInstanced(vertexCount, 1, 0, 0);
        }

        void DrawIndexed(ID3D12GraphicsCommandList* cmdList)
        {
            cmdList->IASetVertexBuffers(0, 1, vertexBuffer.GetBufferViewPtr());
            cmdList->IASetIndexBuffer(indexBuffer.GetBufferViewPtr());
            cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
        }

        void DrawIndexedInstanced(ID3D12GraphicsCommandList* cmdList, UINT instanceCount)
        {
            cmdList->IASetVertexBuffers(0, 1, vertexBuffer.GetBufferViewPtr());
            cmdList->IASetIndexBuffer(indexBuffer.GetBufferViewPtr());
            cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            cmdList->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
        }
    };

    struct GBufferInstanceData
    {
        DirectX::XMFLOAT4X4 worldMatrix;
    };

    struct MaterialMeshBatchKey
    {
        ECS::Material* material = nullptr;
        ECS::GpuMesh* mesh = nullptr;

        bool operator==(const MaterialMeshBatchKey& other) const
        {
            return material == other.material &&
                mesh == other.mesh;
        }
    };

    struct BatchKey
    {
        GpuMesh* mesh;
        Material* mat;

        bool operator==(const BatchKey& other) const
        {
            return mesh == other.mesh && mat == other.mat;
        }
    };

    struct BatchKeyHash
    {
        size_t operator()(const BatchKey& k) const
        {
            size_t h1 = std::hash<void*>()(k.mesh);
            size_t h2 = std::hash<void*>()(k.mat);
            return h1 ^ (h2 * 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
        }
    };

    struct BatchEntry
    {
        entt::entity representative = entt::null;
        std::vector<DirectX::XMFLOAT4X4> worldMatrices;
    };
}
