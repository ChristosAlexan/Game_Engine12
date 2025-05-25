#pragma once
#include "Shapes12.h"

class RectShape12 : public Shapes12
{
public:
	RectShape12();
	virtual void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) override;
	virtual void UploadGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::vector<Vertex>& vertices, std::vector<DWORD>& indices) override;
	virtual void Draw(ID3D12GraphicsCommandList* commandList) override;

};

