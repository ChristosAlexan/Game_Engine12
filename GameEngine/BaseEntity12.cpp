#include "BaseEntity12.h"

BaseEntity12::BaseEntity12()
{
}

void BaseEntity12::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
}

void BaseEntity12::Draw(ID3D12GraphicsCommandList* commandList, DynamicUploadBuffer* dynamicCB, Camera& camera)
{
    CB_VS_SimpleShader vsCB = {};
    vsCB.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
    vsCB.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());
    vsCB.worldMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z));
    CB_PS_SimpleShader psCB = {};

    psCB.lightPos = DirectX::XMFLOAT4(3.0f,5.0f,1.0f,1.0f);
    psCB.color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    commandList->SetGraphicsRootConstantBufferView(0, dynamicCB->Allocate(vsCB));
    commandList->SetGraphicsRootConstantBufferView(1, dynamicCB->Allocate(psCB));
}
