#include "CubeMap.h"
#include "ConstantBufferTypes.h"
#include "Camera.h"
#include "DX12.h"

CubeMap::CubeMap()
{

}

void CubeMap::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* commandAllocator,
	ID3D12DescriptorHeap* sharedRsvHeap, DescriptorAllocator* descriptorAllocator)
{
	m_cubeShape.Initialize(device, cmdList);
	m_cubeShape.pos = DirectX::XMFLOAT3(0, 0, 0);
	m_cubeShape.scale = DirectX::XMFLOAT3(1, 1, 1);
	m_cubeShape.rot = DirectX::XMFLOAT3(0, 0, 0);
	hdr_map1.Initialize(device, cmdList, descriptorAllocator, "Data/HDR/voortrekker_interior_2k.hdr");
	m_cubemapTexture.InitializeCubeMap(device, cmdList, commandAllocator, sharedRsvHeap, descriptorAllocator, 512, 512);
}

RenderTargetTexture& CubeMap::GetCubeMapRenderTargetTexture()
{
	return m_cubemapTexture;
}

void CubeMap::ResetRenderTarget(ID3D12GraphicsCommandList* cmdList)
{
	m_cubemapTexture.Reset(cmdList);
}

void CubeMap::RenderDebug(DX12& dx12, Camera& camera, UINT rootParameterIndex)
{
	m_cubemapTexture.TransitionToRTV(dx12.GetCmdList());
	ID3D12DescriptorHeap* heaps[] = { dx12.GetSharedSrvHeap() };
	dx12.GetCmdList()->SetDescriptorHeaps(1, heaps);

	// Set render target
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(dx12.GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), dx12.frameIndex, dx12.rtvDescriptorSize);
	// Set depth-stencil
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dx12.dsvHeap->GetCPUDescriptorHandleForHeapStart();
	dx12.GetCmdList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	dx12.GetCmdList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	dx12.GetCmdList()->ClearDepthStencilView(
		dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f,
		0,
		0,
		nullptr
	);
	dx12.GetCmdList()->SetPipelineState(dx12.pipelineState_CubemapDebug.Get());
	//dx12.GetCmdList()->SetPipelineState(dx12.pipelineState_Cubemap.Get());
	// Transition to SRV


	m_cubemapTexture.TransitionToSRV(dx12.GetCmdList());
	dx12.GetCmdList()->SetGraphicsRootDescriptorTable(rootParameterIndex, m_cubemapTexture.GetSrvGpuHandle(0));
	//dx12.GetCmdList()->SetGraphicsRootDescriptorTable(8, hdr_map1.GetHDRtexture().GetGPUHandle());

	DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&camera.pos);
	DirectX::XMVECTOR rot_vec = DirectX::XMVectorZero();
	DirectX::XMVECTOR scale_vec = DirectX::XMLoadFloat3(&m_cubeShape.scale);

	DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(scale_vec);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(rot_vec);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(pos_vec);

	DirectX::XMMATRIX worldMatrix = S * R * T;

	DirectX::XMMATRIX proj = camera.GetProjectionMatrix();
	DirectX::XMMATRIX view = camera.GetViewMatrix();
	CB_VS_SimpleShader vsCB = {};
	vsCB.viewMatrix = DirectX::XMMatrixTranspose(view);
	vsCB.projectionMatrix = DirectX::XMMatrixTranspose(proj);
	vsCB.worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);
	if (dx12.dynamicCB)
	{
		dx12.GetCmdList()->SetGraphicsRootConstantBufferView(0, dx12.dynamicCB->Allocate(vsCB));
	}

	m_cubeShape.Draw(dx12.GetCmdList());
}

void CubeMap::Render(DX12& dx12, Camera& camera, ID3D12PipelineState* pipelineState, UINT rootParameterIndex)
{
	using namespace DirectX;

	const XMVECTOR directions[6] = {
		XMVectorSet(+1,  0,  0, 0),  // +X
		XMVectorSet(-1,  0,  0, 0),  // -X
		XMVectorSet(0, +1,  0, 0),  // +Y
		XMVectorSet(0, -1,  0, 0),  // -Y
		XMVectorSet(0,  0, +1, 0),  // +Z
		XMVectorSet(0,  0, -1, 0)   // -Z
	};

	const XMVECTOR ups[6] = {
		XMVectorSet(0, 1, 0, 0),  // +X
		XMVectorSet(0, 1, 0, 0),  // -X
		XMVectorSet(0, 0, -1, 0), // +Y
		XMVectorSet(0, 0,  1, 0), // -Y
		XMVectorSet(0, 1, 0, 0),  // +Z
		XMVectorSet(0, 1, 0, 0),  // -Z
	};

	ID3D12DescriptorHeap* heaps[] = { dx12.GetSharedSrvHeap() };
	dx12.GetCmdList()->SetDescriptorHeaps(1, heaps);
	dx12.GetCmdList()->SetPipelineState(dx12.pipelineState_Cubemap.Get());

	float aspect = static_cast<float>(m_cubemapTexture.m_width)/ static_cast<float>(m_cubemapTexture.m_height);
	float nearZ = 0.01f;
	float farZ = 1000.0f;
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspect, nearZ, farZ);
	DirectX::XMVECTOR position = DirectX::XMVectorZero();
	D3D12_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(m_cubemapTexture.m_width);
	viewport.Height = static_cast<float>(m_cubemapTexture.m_height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = m_cubemapTexture.m_width;
	scissorRect.bottom = m_cubemapTexture.m_height;

	dx12.GetCmdList()->RSSetViewports(1, &viewport);
	dx12.GetCmdList()->RSSetScissorRects(1, &scissorRect);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dx12.dsvHeap->GetCPUDescriptorHandleForHeapStart();

	dx12.GetCmdList()->SetGraphicsRootDescriptorTable(rootParameterIndex, hdr_map1.GetHDRtexture().GetGPUHandle());

	for (uint32_t face = 0; face < 6; ++face)
	{
		dx12.GetCmdList()->OMSetRenderTargets(1, &m_cubemapTexture.m_rtvHandles[face], FALSE, &dsvHandle);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		dx12.GetCmdList()->ClearRenderTargetView(m_cubemapTexture.m_rtvHandles[face], clearColor, 0, nullptr);
		dx12.GetCmdList()->ClearDepthStencilView(
			dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,
			0,
			0,
			nullptr
		);

		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(position, position + directions[face], ups[face]);
		CB_VS_SimpleShader vsCB = {};
		vsCB.viewMatrix = DirectX::XMMatrixTranspose(view);
		vsCB.projectionMatrix = DirectX::XMMatrixTranspose(proj);
		vsCB.worldMatrix = DirectX::XMMatrixIdentity();
		if (dx12.dynamicCB)
		{
			dx12.GetCmdList()->SetGraphicsRootConstantBufferView(0, dx12.dynamicCB->Allocate(vsCB));
		}

		m_cubeShape.Draw(dx12.GetCmdList());
	}

	m_cubemapTexture.TransitionToSRV(dx12.GetCmdList());
}