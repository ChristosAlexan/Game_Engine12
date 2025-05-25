#pragma once
#include "DX12Includes.h"
#include "VertexBuffer12.h"
#include "IndexBuffer12.h"
#include <vector>
#include "Vertex.h"

namespace ECS
{
    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<DWORD> indices;
    };

    struct Mesh12 {
        VertexBuffer12<Vertex> vertexBuffer;
        IndexBuffer12 indexBuffer;
        uint32_t indexCount = 0;

        void Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const MeshData& data) {
            vertexBuffer.Initialize(device, cmdList, data.vertices.data(), data.vertices.size());
            indexBuffer.Initialize(device, cmdList, data.indices.data(), (uint32_t)data.indices.size());
            indexCount = static_cast<uint32_t>(data.indices.size());
        }

        void Draw(ID3D12GraphicsCommandList* cmdList) {
            cmdList->IASetVertexBuffers(0, 1, &vertexBuffer.vbView);
            cmdList->IASetIndexBuffer(&indexBuffer.ibView);
            cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
        }
    };
}
