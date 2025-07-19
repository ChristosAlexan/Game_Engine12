#pragma once

#include "DX12Includes.h"
#include "DescriptorAllocator.h"
#include "Camera.h"
#include "RectShape12.h"
#include "CubeShape12.h"
#include "DXCShaderCompiler.h"
#include "ConstantBufferTypes.h"
#include "DynamicUploadBuffer.h"
#include "AppTimer.h"
#include "GFXGui.h"
#include "RenderTargetTexture.h"
#include "SceneManager.h"

class DX12
{
public:
	DX12();
	~DX12();
	void WaitForGPU(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, HANDLE fenceEvent, UINT64& fenceValue);
	void Initialize(HWND hwnd, int& width, int& height);
	void CreateDeviceAndFactory();
	void CreateCommandObjects();
	void CreateSwapChainAndRTVs(HWND& hwnd, int& width, int& height);
	void CreateFenceAndSyncObjects();
	void CreateSamplerStates();
	void CreateRootSignature(CD3DX12_ROOT_SIGNATURE_DESC& rootSigDesc);
	void CreateDescriptorHeaps();
	void InitializeShaders();
	void CreatePSO(IDxcBlob* vsBlob, IDxcBlob* psBlob, Microsoft::WRL::ComPtr<ID3D12PipelineState>& PSO_pipeline, const D3D12_INPUT_ELEMENT_DESC* inputLayout, const UINT size, 
		const UINT num_renderTargets, const DXGI_FORMAT* formats, D3D12_CULL_MODE cull_mode = D3D12_CULL_MODE_BACK);
	void CreateDepthStencilBuffer(int& width, int& height);
	void InitializeBuffers();
	void TransitionBackBufferToRTV();
	void TransitionBackBufferToPresent();
	void SetRenderTargetToBackBuffer();
	void StartRenderFrame(ECS::SceneManager* sceneManager, GFXGui& gui, Camera& camera, int width, int height, float& dt);
	void EndRenderFrame(ECS::SceneManager* sceneManager, GFXGui& gui, Camera& camera, int width, int height, float& dt);

	void ResetCommands();
	void ResetCommandList();
	void SubmitCommand();
	void InitDescAllocator(ID3D12DescriptorHeap* heap);

	ID3D12CommandQueue* GetCommandQueue() const;
	ID3D12Device* GetDevice() const;
	ID3D12GraphicsCommandList* GetCmdList() const;
	ID3D12CommandAllocator* GetCommandAllocator() const;
	ID3D12DescriptorHeap* GetRtvHeap() const;
	ID3D12DescriptorHeap* GetSharedSrvHeap() const;
	DescriptorAllocator* GetDescriptorAllocator() const;

public:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	DXCShaderCompiler shaderCompiler;
	std::unique_ptr<DynamicUploadBuffer> dynamicCB;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	UINT frameIndex = 0;
	UINT rtvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, pipelineState_2D, pipelineState_Gbuffer, pipelineState_Cubemap, pipelineState_CubemapDebug;
private:
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets[2];
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	std::unique_ptr<DescriptorAllocator> m_descAllocator;
	
	Microsoft::WRL::ComPtr<IDXGIFactory7> factory;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;


	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sharedSrvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
	CD3DX12_RESOURCE_BARRIER m_barrier;

	// SAMPLE DESCS
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc;



	UINT64 fenceValue = 0;
	HANDLE fenceEvent = nullptr;



	AppTimer timer;
};

