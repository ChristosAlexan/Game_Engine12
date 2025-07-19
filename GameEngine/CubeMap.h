#pragma once
#include "RenderTargetTexture.h"
#include "DynamicUploadBuffer.h"
#include "CubeShape12.h"
#include "HDR_IMAGE.h"

class Camera;
class DX12;
class CubeMap
{
public:
	CubeMap();
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* commandAllocator,
		ID3D12DescriptorHeap* sharedRsvHeap, DescriptorAllocator* descriptorAllocator);
	RenderTargetTexture& GetCubeMapRenderTargetTexture();
	void ResetRenderTarget(ID3D12GraphicsCommandList* cmdList);
	void RenderDebug(DX12& dx12, Camera& camera, UINT rootParameterIndex);
	void Render(DX12& dx12, Camera& camera, ID3D12PipelineState* pipelineState, UINT rootParameterIndex);

	CubeShape12 m_cubeShape;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;



	HDR_IMAGE hdr_map1;
	bool bRender = true;
private:
	RenderTargetTexture m_cubemapTexture;
	
};