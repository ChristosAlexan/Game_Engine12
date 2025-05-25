#pragma once
#include "BaseEntity12.h"
#include "CubeShape12.h"

class CubeEntity12 : public BaseEntity12
{
public:
	CubeEntity12();
	virtual void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) override;
	virtual void Draw(ID3D12GraphicsCommandList* commandList, DynamicUploadBuffer* dynamicCB, Camera& camera) override;

private:
	CubeShape12 m_cube;
};

