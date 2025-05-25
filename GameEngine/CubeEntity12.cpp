#include "CubeEntity12.h"

CubeEntity12::CubeEntity12()
{
}

void CubeEntity12::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	BaseEntity12::Initialize(device, commandList);

	m_cube.Initialize(device, commandList);
}

void CubeEntity12::Draw(ID3D12GraphicsCommandList* commandList, DynamicUploadBuffer* dynamicCB, Camera& camera)
{
	BaseEntity12::Draw(commandList, dynamicCB, camera);
	m_cube.Draw(commandList);
}
