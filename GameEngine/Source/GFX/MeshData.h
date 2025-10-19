#pragma once
#include "DX12Includes.h"
#include "VertexBuffer12.h"
#include "IndexBuffer12.h"
#include <vector>
#include "Vertex.h"
#include "MaterialECS.h"
#include "RayTraceData.h"
#include "StructuredBuffer.h"

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

    struct GpuMesh 
    {
        D3D12_GPU_DESCRIPTOR_HANDLE skinningGpuHandleIn{};
     
        VertexBuffer12<Vertex> vertexBuffer;
        IndexBuffer12 indexBuffer;
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
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
    };
}
