#include "CubeShape12.h"

CubeShape12::CubeShape12()
{
}

void CubeShape12::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	std::vector<Vertex> vertices;

	// +Y (Top face)
	vertices.push_back(Vertex(-1, 1, -1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, -1));
	vertices.push_back(Vertex(1, 1, -1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, -1));
	vertices.push_back(Vertex(1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, -1));
	vertices.push_back(Vertex(-1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, -1));

	// -Y (Bottom face)
	vertices.push_back(Vertex(-1, -1, 1, 0, 0, 0, -1, 0, 1, 0, 0, 0, 0, 1));
	vertices.push_back(Vertex(1, -1, 1, 1, 0, 0, -1, 0, 1, 0, 0, 0, 0, 1));
	vertices.push_back(Vertex(1, -1, -1, 1, 1, 0, -1, 0, 1, 0, 0, 0, 0, 1));
	vertices.push_back(Vertex(-1, -1, -1, 0, 1, 0, -1, 0, 1, 0, 0, 0, 0, 1));

	// +X (Right face)
	vertices.push_back(Vertex(1, 1, 1, 0, 0, 1, 0, 0, 0, 0, -1, 0, 1, 0));
	vertices.push_back(Vertex(1, 1, -1, 1, 0, 1, 0, 0, 0, 0, -1, 0, 1, 0));
	vertices.push_back(Vertex(1, -1, -1, 1, 1, 1, 0, 0, 0, 0, -1, 0, 1, 0));
	vertices.push_back(Vertex(1, -1, 1, 0, 1, 1, 0, 0, 0, 0, -1, 0, 1, 0));

	// -X (Left face)
	vertices.push_back(Vertex(-1, 1, -1, 0, 0, -1, 0, 0, 0, 0, 1, 0, 1, 0));
	vertices.push_back(Vertex(-1, 1, 1, 1, 0, -1, 0, 0, 0, 0, 1, 0, 1, 0));
	vertices.push_back(Vertex(-1, -1, 1, 1, 1, -1, 0, 0, 0, 0, 1, 0, 1, 0));
	vertices.push_back(Vertex(-1, -1, -1, 0, 1, -1, 0, 0, 0, 0, 1, 0, 1, 0));

	// +Z (Front face)
	vertices.push_back(Vertex(-1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0));
	vertices.push_back(Vertex(1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0));
	vertices.push_back(Vertex(1, -1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0));
	vertices.push_back(Vertex(-1, -1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0));

	// -Z (Back face)
	vertices.push_back(Vertex(1, 1, -1, 0, 0, 0, 0, -1, -1, 0, 0, 0, 1, 0));
	vertices.push_back(Vertex(-1, 1, -1, 1, 0, 0, 0, -1, -1, 0, 0, 0, 1, 0));
	vertices.push_back(Vertex(-1, -1, -1, 1, 1, 0, 0, -1, -1, 0, 0, 0, 1, 0));
	vertices.push_back(Vertex(1, -1, -1, 0, 1, 0, 0, -1, -1, 0, 0, 0, 1, 0));
	
	std::vector<DWORD> indices = {
	0,2,1, 0,3,2,    // Top face
	4,6,5, 4,7,6,    // Bottom face
	8,10,9, 8,11,10, // Right face
	12,14,13, 12,15,14, // Left face
	16,18,17, 16,19,18, // Front face
	20,22,21, 20,23,22  // Back face
	};

	UploadGeometry(device, commandList, vertices, indices);
}

void CubeShape12::UploadGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::vector<Vertex>& vertices, std::vector<DWORD>& indices)
{
	Shapes12::UploadGeometry(device, commandList, vertices, indices);
}

void CubeShape12::Draw(ID3D12GraphicsCommandList* commandList)
{
	Shapes12::Draw(commandList);
}
