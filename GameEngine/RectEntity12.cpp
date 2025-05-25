#include "RectEntity12.h"

RectEntity12::RectEntity12()
{
}

void RectEntity12::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	BaseEntity12::Initialize(device, commandList);

	m_rect.Initialize(device, commandList);
}

void RectEntity12::Draw(ID3D12GraphicsCommandList* commandList, DynamicUploadBuffer* dynamicCB, Camera& camera)
{
	m_position = DirectX::XMFLOAT4(-5, 0, 0, 1);
	BaseEntity12::Draw(commandList, dynamicCB, camera);
	m_rect.Draw(commandList);
}
