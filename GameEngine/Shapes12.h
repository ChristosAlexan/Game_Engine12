#pragma once
#include "DX12Includes.h"
#include <DirectXMath.h>
#include "Vertex.h"
#include "VertexBuffer12.h"
#include "IndexBuffer12.h"

class Shapes12
{
public:
	Shapes12();
	virtual void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) = 0;
	virtual void UploadGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::vector<Vertex>& vertices, std::vector<DWORD>& indices);
	virtual void Draw(ID3D12GraphicsCommandList* commandList);
public:
	DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 rot = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(0, 0, 0);

protected:
	VertexBuffer12<Vertex> vertexBuffer;
	IndexBuffer12 indexBuffer;

	UINT indexCount = 0;
};

