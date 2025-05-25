#pragma once
#include "BaseEntity12.h"
#include "RectShape12.h"

class RectEntity12 : public BaseEntity12
{
public:
	RectEntity12();
	virtual void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) override;
	virtual void Draw(ID3D12GraphicsCommandList* commandList, DynamicUploadBuffer* dynamicCB, Camera& camera) override;

private:
	RectShape12 m_rect;
};

