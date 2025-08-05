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

	uint32_t GetScreenWidth();
	uint32_t GetScreenHeight();
	void CreateUploadBuffer(UINT64 size, const void* initData, Microsoft::WRL::ComPtr<ID3D12Resource>& defaultBuffer, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRaytracingInstanceUploadBuffer(UINT64 size, const void* initData);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRayTracingBuffer(UINT64 size, D3D12_RESOURCE_STATES initialState);
	void WaitForGPU(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, HANDLE fenceEvent, UINT64& fenceValue);
	void Initialize(HWND hwnd, int& width, int& height);
	void CreateDeviceAndFactory();
	void CreateCommandObjects();
	void CreateSwapChainAndRTVs(HWND& hwnd, int& width, int& height);
	void CreateFenceAndSyncObjects();
	void CreateSamplerStates();
	void CreateRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc);
	void CreateDescriptorHeaps();
	void InitializeShaders();
	void CreatePSO(IDxcBlob* vsBlob, IDxcBlob* psBlob, Microsoft::WRL::ComPtr<ID3D12PipelineState>& PSO_pipeline, const D3D12_INPUT_ELEMENT_DESC* inputLayout, const UINT size, 
		const UINT num_renderTargets, const DXGI_FORMAT* formats, D3D12_CULL_MODE cull_mode = D3D12_CULL_MODE_BACK);
	void CreateRTPSO(IDxcBlob* rayTracingBlob);
	void CreateSBT();
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
	ID3D12Device5* GetDevice() const;
	ID3D12GraphicsCommandList5* GetCmdList() const;
	ID3D12CommandAllocator* GetCommandAllocator() const;
	ID3D12DescriptorHeap* GetRtvHeap() const;
	ID3D12DescriptorHeap* GetSharedSrvHeap() const;
	DescriptorAllocator* GetDescriptorAllocator() const;
	ID3D12RootSignature* GetRootSignature() const;
	void DispatchRaytracing();

public:
	DXCShaderCompiler shaderCompiler;
	std::unique_ptr<DynamicUploadBuffer> dynamicCB;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	UINT frameIndex = 0;
	UINT rtvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, pipelineState_2D, pipelineState_Gbuffer, 
		pipelineState_Cubemap, pipelineState_CubemapDebug, pipelineState_IrradianceConv, pipelineState_Prefilter, pipelineState_Brdf, pipelineState_raytracingRenderTarget;
	Microsoft::WRL::ComPtr<ID3D12StateObject> rtpso; // Ray tracing state object




	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue = 0;
	HANDLE fenceEvent = nullptr;

	std::unique_ptr<Texture12> m_textureUAV; // Ray tracing output

private:
	uint32_t m_screenWidth, m_screenHeight;

	Microsoft::WRL::ComPtr<ID3D12Device5> device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> commandList;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets[2];

	std::unique_ptr<DescriptorAllocator> m_descAllocator;
	
	Microsoft::WRL::ComPtr<IDXGIFactory7> factory;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;


	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sharedSrvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
	CD3DX12_RESOURCE_BARRIER m_barrier;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_rayGenBuffer, m_missBuffer, m_hitGroupBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_rayGenUploadBuffer, m_missUploadBuffer, m_hitGroupUploadBuffer;
	// SAMPLE DESCS
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc;

	//Ray tracing descs
	D3D12_DISPATCH_RAYS_DESC dispatchDesc;



	AppTimer timer;
};

