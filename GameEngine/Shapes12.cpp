#include "Shapes12.h"

Shapes12::Shapes12()
{
}

void Shapes12::UploadGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	vertexBuffer.Initialize(device, commandList, vertices.data(), vertices.size());
	indexBuffer.Initialize(device, commandList, indices.data(), static_cast<UINT>(indices.size()));

	indexCount = static_cast<UINT>(indices.size());
}

void Shapes12::Draw(ID3D12GraphicsCommandList* commandList)
{
	// Bind the vertex and index buffers
	commandList->IASetVertexBuffers(0, 1, &vertexBuffer.vbView);
	commandList->IASetIndexBuffer(&indexBuffer.ibView);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Issue the draw call
	commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}
