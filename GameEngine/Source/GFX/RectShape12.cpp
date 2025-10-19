#include "RectShape12.h"

RectShape12::RectShape12()
{
}

void RectShape12::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	std::vector<Vertex> vertices;
	vertices.push_back(Vertex(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(1.0f, 1.0f, 0.0f, 1.0f, 0.0f));
	vertices.push_back(Vertex(1.0f, -1.0f, 0.0f, 1.0f, 1.0f));

	std::vector<uint32_t> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	UploadGeometry(device, commandList, vertices, indices);
}

void RectShape12::UploadGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	Shapes12::UploadGeometry(device, commandList, vertices, indices);
}

void RectShape12::Draw(ID3D12GraphicsCommandList* commandList)
{
	Shapes12::Draw(commandList);
}
