#pragma once
#include "DX12Includes.h"
#include "VertexBuffer12.h"
#include "IndexBuffer12.h"
#include <vector>
#include "Vertex.h"
#include "MaterialECS.h"

namespace ECS
{
    struct MeshData 
    {
        std::vector<Vertex> vertices;
        std::vector<DWORD> indices;
    };

    struct GpuMesh 
    {
        VertexBuffer12<Vertex> vertexBuffer;
        IndexBuffer12 indexBuffer;
        uint32_t indexCount = 0;
        MeshData cpuMesh;

        void Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) 
        {
            vertexBuffer.Initialize(device, cmdList, cpuMesh.vertices.data(), cpuMesh.vertices.size());
            indexBuffer.Initialize(device, cmdList, cpuMesh.indices.data(), (uint32_t)cpuMesh.indices.size());
            indexCount = static_cast<uint32_t>(cpuMesh.indices.size());
        }

        void Draw(ID3D12GraphicsCommandList* cmdList) 
        {
            cmdList->IASetVertexBuffers(0, 1, &vertexBuffer.vbView);
            cmdList->IASetIndexBuffer(&indexBuffer.ibView);
            cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
        }
    };
}
