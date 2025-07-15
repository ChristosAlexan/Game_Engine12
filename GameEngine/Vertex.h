#pragma once
#include <DirectXMath.h>

struct Vertex
{
	Vertex() {

	}

	Vertex(float x, float y)
		:pos(x, y, 0.0f)
	{

	}
	Vertex(float x, float y, float z)
		:pos(x, y, z)
	{

	}
	Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v)
		: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz)

	{

	}

	Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz, float tX, float tY, float tZ, float tW, float bX, float bY, float bZ)
		: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz), tangent(tX, tY, tZ, tW), binormal(bX, bY, bZ)

	{

	}
	Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz, float tX, float tY, float tZ, float tW, float bX, float bY, float bZ, float r, float g, float b)
		: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz), tangent(tX, tY, tZ, tW), binormal(bX, bY, bZ)
	{

	}
	Vertex(float x, float y, float u, float v)
		:pos(x, y, 0.0f), texCoord(u, v)
	{

	}
	Vertex(float x, float y, float z, float u, float v)
		:pos(x, y, z), texCoord(u, v)
	{

	}

	Vertex(float x, float y, float z, float r, float g, float b)
		:pos(x, y, z)
	{

	}

	DirectX::XMFLOAT3 pos = { 0,0,0 };
	DirectX::XMFLOAT2 texCoord = { 0,0 };
	DirectX::XMFLOAT3 normal = { 0,0,0 };

	DirectX::XMFLOAT4 tangent = { 0,0,0,0 };
	DirectX::XMFLOAT3 binormal = { 0,0,0 };

	float boneWeights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	uint32_t boneIndices[4] = {0,0,0,0};
	
};


