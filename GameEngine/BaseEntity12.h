#pragma once
#include "DX12Includes.h"
#include "ConstantBufferTypes.h"
#include "Camera.h"
#include "DynamicUploadBuffer.h"

class BaseEntity12
{
public:
	BaseEntity12();
	virtual void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	virtual void Draw(ID3D12GraphicsCommandList* commandList, DynamicUploadBuffer* dynamicCB,  Camera& camera);

protected:
	DirectX::XMMATRIX m_worldMatrix;
	DirectX::XMFLOAT4 m_position;
	DirectX::XMFLOAT4 m_scale;
	DirectX::XMFLOAT4 m_rotation;
};

