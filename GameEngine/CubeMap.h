#pragma once
#include "RenderTargetTexture.h"
#include "DynamicUploadBuffer.h"
#include "CubeShape12.h"
#include <d3d12.h>

class Camera;
class DX12;
class CubeMap
{
public:
	CubeMap();
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandAllocator* commandAllocator,
		ID3D12DescriptorHeap* sharedRsvHeap, DescriptorAllocator* descriptorAllocator, const uint32_t width, const uint32_t height, const UINT16 mipLevels = 1);
	RenderTargetTexture& GetCubeMapRenderTargetTexture();
	void ResetRenderTarget(ID3D12GraphicsCommandList* cmdList);
	void RenderDebug(DX12& dx12, Camera& camera, UINT rootParameterIndex);
	void Render(DX12& dx12, Camera& camera, ID3D12PipelineState* pipelineState, const UINT rootParameterIndex, const D3D12_GPU_DESCRIPTOR_HANDLE& gpu_handle);
	void RenderMips(DX12& dx12, Camera& camera, ID3D12PipelineState* pipelineState, const UINT rootParameterIndex, const D3D12_GPU_DESCRIPTOR_HANDLE& gpu_handle);

	CubeShape12 m_cubeShape;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

	bool bRender = true;
private:
	RenderTargetTexture m_cubemapTexture;
	
};