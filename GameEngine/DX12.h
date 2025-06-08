#pragma once

#include "DX12Includes.h"
#include "Camera.h"
#include "RectShape12.h"
#include "CubeShape12.h"
#include "DXCShaderCompiler.h"
#include "ConstantBufferTypes.h"
#include "DynamicUploadBuffer.h"
#include "AppTimer.h"
#include "SceneManager.h"
#include "GFXGui.h"

class DX12
{
public:
	DX12();
	void Initialize(HWND hwnd, Camera& camera, int& width, int& height);
	void CreateDeviceAndFactory();
	void CreateCommandObjects();
	void CreateSwapChainAndRTVs(HWND& hwnd, int& width, int& height);
	void CreateFenceAndSyncObjects();
	void CreateSamplerStates();
	void CreateRootSignature(CD3DX12_ROOT_SIGNATURE_DESC& rootSigDesc);
	void CreateDescriptorHeaps();
	void InitializeShaders();
	void CreatePSO(IDxcBlob* vsBlob, IDxcBlob* psBlob);
	void CreateDepthStencilBuffer(int& width, int& height);
	void InitializeConstantBuffers();
	void RenderFrame(Camera& camera, int width, int height, float& dt);
private:
	void CreateScene(Camera& camera, int& width, int& height);
	void ResetCommandAllocator();
	void SubmitCommand();

public:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	DXCShaderCompiler shaderCompiler;
private:
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets[2];
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<IDXGIFactory7> factory;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sharedHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;

	// SAMPLE DESCS
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc;


	UINT frameIndex = 0;
	UINT64 fenceValue = 0;
	HANDLE fenceEvent = nullptr;
	UINT rtvDescriptorSize;

	std::unique_ptr<DynamicUploadBuffer> dynamicCB;
	AppTimer timer;

	ECS::SceneManager m_sceneManager;
	GFXGui m_gui;
};

