
#define INITGUID
#include <d3d12.h>
#include "DX12.h"
#include <stdexcept>
#include "ErrorLogger.h"
#include "GFX_MACROS.h"

DX12::DX12()
{
    timer.Start();
}

DX12::~DX12()
{
    // Wait for the gpu to release the resources
    WaitForGPU(commandQueue.Get(), fence.Get(), fenceEvent, fenceValue);
    m_textureUAV.reset();
    m_rayGenBuffer.Reset();
    m_missBuffer.Reset();
    m_hitGroupBuffer.Reset();
    swapChain.Reset();
    commandQueue.Reset();
    fence.Reset();
    device.Reset();
}

uint32_t DX12::GetScreenWidth()
{
    return m_screenWidth;
}

uint32_t DX12::GetScreenHeight()
{
    return m_screenHeight;
}

void DX12::CreateUploadBuffer(UINT64 size, const void* initData, Microsoft::WRL::ComPtr<ID3D12Resource>& defaultBuffer, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
    // Create a default heap buffer
    CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size, D3D12_RESOURCE_FLAG_NONE);

    device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&defaultBuffer)
    );

    // Create an upload heap buffer (CPU accessible)
    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)
    );

    // Map and write the init data to upload heap
    void* pData = nullptr;
    uploadBuffer->Map(0, nullptr, &pData);
    memcpy(pData, initData, size);
    uploadBuffer->Unmap(0, nullptr);

    // Schedule a copy to the default heap
    commandList->CopyResource(defaultBuffer.Get(), uploadBuffer.Get());
}

Microsoft::WRL::ComPtr<ID3D12Resource> DX12::CreateRaytracingInstanceUploadBuffer(UINT64 size, const void* initData)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&buffer)
    );
    COM_ERROR_IF_FAILED(hr, "Failed to create Committed Resource!");

    // Copy data directly
    void* mapped = nullptr;
    buffer->Map(0, nullptr, &mapped);
    memcpy(mapped, initData, size);
    buffer->Unmap(0, nullptr);

    return buffer;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DX12::CreateRayTracingBuffer(UINT64 size, D3D12_RESOURCE_STATES initialState)
{
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc = { 1, 0 };
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        initialState,
        nullptr,
        IID_PPV_ARGS(&buffer)
    );

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create UAV buffer");
    }

    return buffer;
}

void DX12::WaitForGPU(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, HANDLE fenceEvent, UINT64& fenceValue)
{
    const UINT64 fenceSignal = ++fenceValue;
    commandQueue->Signal(fence, fenceSignal);

    if (fence->GetCompletedValue() < fenceSignal)
    {
        fence->SetEventOnCompletion(fenceSignal, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

void DX12::ResetCommands()
{
    // Reset command list before issuing GPU uploads
    commandAllocator->Reset();
    ResetCommandList();
}

void DX12::ResetCommandList()
{
    commandList->Reset(commandAllocator.Get(), nullptr);
}
void DX12::SubmitCommand()
{
    // Finish and execute upload
    commandList->Close();
    ID3D12CommandList* commandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
    const UINT64 currentFence = fenceValue;

    // Ensure upload completes before rendering
    WaitForGPU(commandQueue.Get(), fence.Get(), fenceEvent, fenceValue);
}

void DX12::InitDescAllocator(ID3D12DescriptorHeap* heap)
{
    m_descAllocator = std::make_unique<DescriptorAllocator>(device.Get(), heap, 512, true);
}

ID3D12CommandQueue* DX12::GetCommandQueue() const
{
    return commandQueue.Get();
}

ID3D12Device5* DX12::GetDevice() const
{
    return device.Get();
}

ID3D12GraphicsCommandList5* DX12::GetCmdList() const
{
    return commandList.Get();
}

ID3D12CommandAllocator* DX12::GetCommandAllocator() const
{
    return commandAllocator.Get();
}

ID3D12DescriptorHeap* DX12::GetRtvHeap() const
{
    return rtvHeap.Get();
}

ID3D12DescriptorHeap* DX12::GetSharedSrvHeap() const
{
    return sharedSrvHeap.Get();
}

DescriptorAllocator* DX12::GetDescriptorAllocator() const
{
    return m_descAllocator.get();
}

ID3D12RootSignature* DX12::GetRootSignature() const
{
    return rootSignature.Get();
}

void DX12::DispatchRaytracing()
{
    commandList->SetPipelineState1(rtpso.Get());
    commandList->DispatchRays(&dispatchDesc);
}

void DX12::Initialize(HWND hwnd, int& width, int& height)
{
    m_screenWidth = width;
    m_screenHeight = height;

    CreateDeviceAndFactory();
    CreateCommandObjects();
    CreateSwapChainAndRTVs(hwnd, width, height);
    CreateFenceAndSyncObjects();
    CreateDescriptorHeaps();
    CreateDepthStencilBuffer(width, height);
    CreateSamplerStates();
    InitializeBuffers();
    InitializeShaders();
    InitDescAllocator(sharedSrvHeap.Get());

    ResetCommands();

    CreateSBT();

    m_textureUAV = std::make_unique<Texture12>();
    m_textureUAV->CreateTextureUAV(GetDevice(), GetCmdList(), GetDescriptorAllocator(), width, height);
}

void DX12::CreateDeviceAndFactory()
{
    HRESULT hr;
    UINT dxgiFactoryFlags = 0;
  

#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0;
        DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter);
        ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) 
        {
            continue; // skip software adapters
        }

        hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
            break;
    }
    COM_ERROR_IF_FAILED(hr, "Failed to create device");

    // Check for ray tracing compatibility
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};
    device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData));
    if (featureSupportData.RaytracingTier < D3D12_RAYTRACING_TIER_1_0) 
    {
        throw std::runtime_error("Raytracing not supported on this device.");
    }

}
void DX12::CreateCommandObjects()
{
    // --- Create Command Queue ---
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    COM_ERROR_IF_FAILED(hr, "Failed to create command queue");
 
    // --- Create Command Allocator ---
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    COM_ERROR_IF_FAILED(hr, "Failed to create command allocator");

    // --- Create Command List ---
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    COM_ERROR_IF_FAILED(hr, "Failed to create command list");
   
    commandList->Close();
}

void DX12::CreateSwapChainAndRTVs(HWND& hwnd, int& width, int& height)
{
    // --- Create RTV Descriptor Heap ---
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    HRESULT hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
    COM_ERROR_IF_FAILED(hr, "Failed to create RTV descriptor heap");
  
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // --- Create Swap Chain ---
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapChain;
    hr = factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &tempSwapChain);
    COM_ERROR_IF_FAILED(hr, "Failed to create swap chain");

    tempSwapChain.As(&swapChain);
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    // --- Create Render Target Views ---
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < 2; ++i) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }
}

void DX12::CreateFenceAndSyncObjects()
{
    HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    COM_ERROR_IF_FAILED(hr, "Failed to create fence");

    fenceValue = 1;

    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == nullptr) {
        COM_ERROR_IF_FAILED(hr, "Failed to create fence event");
    }
}

void DX12::CreateSamplerStates()
{
    samplerDesc = CD3DX12_STATIC_SAMPLER_DESC();
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
    samplerDesc.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
}

void DX12::CreateRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc)
{
    // Serialize
    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc;

    HRESULT hr = D3D12SerializeVersionedRootSignature(
        &rootSigDesc,
        &serializedRootSig,
        &errorBlob
    );
    if (FAILED(hr)) {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        COM_ERROR_IF_FAILED(hr, "Failed to serialize root signature");
    }

    // Create
    hr = device->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)
    );
    COM_ERROR_IF_FAILED(hr, "Failed to create root signature");
}

void DX12::CreateDescriptorHeaps()
{
    HRESULT hr;
    // --- Create SRV Descriptor Heap ---
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 512;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&sharedSrvHeap));
    COM_ERROR_IF_FAILED(hr, "Failed to create srv descriptor heap");

    // --- Create DSV Descriptor Heap ---
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&dsvHeap));
    COM_ERROR_IF_FAILED(hr, "Failed to create dsv descriptor heap");
}

void DX12::InitializeShaders()
{
    DXGI_FORMAT default_format8_UNORM = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT default_format16_FLOAT = DXGI_FORMAT_R16G16B16A16_FLOAT;
    {
        DXCShaderCompiler compiler;

        auto vsBlob = compiler.CompileShader(L"VertexShader12.hlsl", L"Main", L"vs_6_7");
        auto psBlob = compiler.CompileShader(L"PixelShader12.hlsl", L"Main", L"ps_6_7");

        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0  },
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0  },
            {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0  },
            { "BONEWEIGHTS",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "BONEINDICES",   0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        UINT layoutSize = _countof(inputLayout);
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState, inputLayout, layoutSize, 1, &default_format16_FLOAT);
        
        // Create Gbuffer pipelineState
        DXGI_FORMAT formats[GBUFFER_TEXTURES_NUM];
        formats[GBUFFER_RENDER_TARGETS_FORMAT_MAPPINGS::ALBEDO] = FORMAT_ALBEDO;
        formats[GBUFFER_RENDER_TARGETS_FORMAT_MAPPINGS::NORMAL] = FORMAT_NORMAL;
        formats[GBUFFER_RENDER_TARGETS_FORMAT_MAPPINGS::ROUGH_METAL] = FORMAT_ROUGH_METAL;
        formats[GBUFFER_RENDER_TARGETS_FORMAT_MAPPINGS::WORLDPOS_DEPTH] = FORMAT_WORLDPOS_DEPTH;

        psBlob = compiler.CompileShader(L"GBufferPS.hlsl", L"Main", L"ps_6_7");
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState_Gbuffer, inputLayout, layoutSize, GBUFFER_TEXTURES_NUM, formats);
    }

    {
        DXCShaderCompiler compiler;

        auto vsBlob = compiler.CompileShader(L"VertexShader_2D.hlsl", L"Main", L"vs_6_7");
        auto psBlob = compiler.CompileShader(L"PixelShader_lightPass.hlsl", L"Main", L"ps_6_7");

        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        UINT layoutSize = _countof(inputLayout);
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState_2D, inputLayout, layoutSize, 1, &default_format16_FLOAT);
    }

    {
        DXCShaderCompiler compiler;

        auto vsBlob = compiler.CompileShader(L"Cubemap_VS.hlsl", L"Main", L"vs_6_7");
        auto psBlob = compiler.CompileShader(L"Cubemap_PS.hlsl", L"Main", L"ps_6_7");

        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        UINT layoutSize = _countof(inputLayout);
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState_Cubemap, inputLayout, layoutSize, 1, &default_format16_FLOAT, D3D12_CULL_MODE_NONE);
    }

    {
        DXCShaderCompiler compiler;

        auto vsBlob = compiler.CompileShader(L"Cubemap_VS.hlsl", L"Main", L"vs_6_7");
        auto psBlob = compiler.CompileShader(L"IrradianceConvolutionPS.hlsl", L"Main", L"ps_6_7");

        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        UINT layoutSize = _countof(inputLayout);
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState_IrradianceConv, inputLayout, layoutSize, 1, &default_format16_FLOAT, D3D12_CULL_MODE_NONE);
    }

    {
        DXCShaderCompiler compiler;

        auto vsBlob = compiler.CompileShader(L"Cubemap_VS.hlsl", L"Main", L"vs_6_7");
        auto psBlob = compiler.CompileShader(L"PrefilterPS.hlsl", L"Main", L"ps_6_7");

        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        UINT layoutSize = _countof(inputLayout);
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState_Prefilter, inputLayout, layoutSize, 1, &default_format16_FLOAT, D3D12_CULL_MODE_NONE);
    }

    {
        DXCShaderCompiler compiler;

        auto vsBlob = compiler.CompileShader(L"CubemapDebug_VS.hlsl", L"Main", L"vs_6_7");
        auto psBlob = compiler.CompileShader(L"CubemapDebug_PS.hlsl", L"Main", L"ps_6_7");

        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        UINT layoutSize = _countof(inputLayout);
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState_CubemapDebug, inputLayout, layoutSize, 1, &default_format16_FLOAT, D3D12_CULL_MODE_NONE);
    }

    {
        DXCShaderCompiler compiler;

        auto vsBlob = compiler.CompileShader(L"VertexShader_2D.hlsl", L"Main", L"vs_6_7");
        auto psBlob = compiler.CompileShader(L"BRDF_PS.hlsl", L"Main", L"ps_6_7");

        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        UINT layoutSize = _countof(inputLayout);
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState_Brdf, inputLayout, layoutSize, 1, &default_format16_FLOAT, D3D12_CULL_MODE_NONE);
    }

    {
        DXCShaderCompiler compiler;
   
        auto vsBlob = compiler.CompileShader(L"VertexShader_2D.hlsl", L"Main", L"vs_6_7");
        auto psBlob = compiler.CompileShader(L"RaytracingPS.hlsl", L"Main", L"ps_6_7");
   
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        UINT layoutSize = _countof(inputLayout);
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState_raytracingRenderTarget, inputLayout, layoutSize, 1, &default_format16_FLOAT, D3D12_CULL_MODE_NONE);
    }

    // Ray tracing shader
    {
        DXCShaderCompiler compiler;
   
        auto rayTraceBlob = compiler.CompileShader(L"RayTracingShader.hlsl", std::wstring{}, L"lib_6_7");
        CreateRTPSO(rayTraceBlob.Get());
    }
}

void DX12::CreatePSO(IDxcBlob* vsBlob, IDxcBlob* psBlob, Microsoft::WRL::ComPtr<ID3D12PipelineState>& PSO_pipeline, const D3D12_INPUT_ELEMENT_DESC* inputLayout, const UINT size, const UINT num_renderTargets, 
    const DXGI_FORMAT* formats, D3D12_CULL_MODE cull_mode)
{
    HRESULT hr;
    
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, size };
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = cull_mode;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = num_renderTargets;
    for(UINT i = 0; i < psoDesc.NumRenderTargets; ++i)
        psoDesc.RTVFormats[i] = formats[i];
    psoDesc.SampleDesc.Count = 1;
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO_pipeline));
    COM_ERROR_IF_FAILED(hr, "Failed to create pipeline state");
}

void DX12::CreateRTPSO(IDxcBlob* rayTracingBlob)
{
    D3D12_EXPORT_DESC exportDescs[3] = {};
    exportDescs[0] = { L"RayGen", nullptr, D3D12_EXPORT_FLAG_NONE };
    exportDescs[1] = { L"Miss", nullptr, D3D12_EXPORT_FLAG_NONE };
    exportDescs[2] = { L"ClosestHit", nullptr, D3D12_EXPORT_FLAG_NONE };

    D3D12_DXIL_LIBRARY_DESC dxilLibDesc = {};
    dxilLibDesc.DXILLibrary.BytecodeLength = rayTracingBlob->GetBufferSize();
    dxilLibDesc.DXILLibrary.pShaderBytecode = rayTracingBlob->GetBufferPointer();

    static LPCWSTR shaderExports[] = { L"RayGen", L"Miss", L"ClosestHit" };
    dxilLibDesc.NumExports = _countof(shaderExports);
    dxilLibDesc.pExports = exportDescs;

    D3D12_STATE_SUBOBJECT libSubobject = {};
    libSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    libSubobject.pDesc = &dxilLibDesc;

    // Create hit group
    D3D12_HIT_GROUP_DESC hitGroupDesc = {};
    hitGroupDesc.ClosestHitShaderImport = L"ClosestHit";
    hitGroupDesc.HitGroupExport = L"MyHitGroup";
    hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

    D3D12_STATE_SUBOBJECT hitGroupSubobject = {};
    hitGroupSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    hitGroupSubobject.pDesc = &hitGroupDesc;

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    shaderConfig.MaxPayloadSizeInBytes = 16;
    shaderConfig.MaxAttributeSizeInBytes = 8; // Two floats for barycentrics

    D3D12_STATE_SUBOBJECT shaderConfigSubobject = {};
    shaderConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    shaderConfigSubobject.pDesc = &shaderConfig;

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = 1; // Increase for accuracy

    D3D12_STATE_SUBOBJECT pipelineConfigSubobject = {};
    pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    pipelineConfigSubobject.pDesc = &pipelineConfig;

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSigDesc = {};
    globalRootSigDesc.pGlobalRootSignature = rootSignature.Get();
    D3D12_STATE_SUBOBJECT globalRootSigSubobject = {};
    globalRootSigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    globalRootSigSubobject.pDesc = &globalRootSigDesc;

    std::vector<D3D12_STATE_SUBOBJECT> subobjects = {
    libSubobject,
    hitGroupSubobject,
    shaderConfigSubobject,
    globalRootSigSubobject,
    pipelineConfigSubobject
    };

    D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
    stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    stateObjectDesc.NumSubobjects = (UINT)subobjects.size();
    stateObjectDesc.pSubobjects = subobjects.data();


   device->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&rtpso));
}

void DX12::CreateSBT()
{
    dispatchDesc = D3D12_DISPATCH_RAYS_DESC();
    dispatchDesc.Width = GetScreenWidth();
    dispatchDesc.Height = GetScreenHeight();
    dispatchDesc.Depth = 1;

    Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> stateObjectProps;
    rtpso.As(&stateObjectProps);
    void* rayGenShaderID = stateObjectProps->GetShaderIdentifier(L"RayGen");

    // Create an upload buffer to hold the RayGen record
    uint8_t rayGenRecord[D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT] = {};
    memcpy(rayGenRecord, rayGenShaderID, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    uint32_t rayGenRecordSize = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
    CreateUploadBuffer(rayGenRecordSize, rayGenRecord, m_rayGenBuffer, m_rayGenUploadBuffer);
    
    dispatchDesc.RayGenerationShaderRecord.StartAddress = m_rayGenBuffer->GetGPUVirtualAddress();
    dispatchDesc.RayGenerationShaderRecord.SizeInBytes = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

    void* missShaderID = stateObjectProps->GetShaderIdentifier(L"Miss");
    uint32_t missRecordSize = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
    uint8_t missRecord[D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT] = {};
    memcpy(missRecord, missShaderID, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
   CreateUploadBuffer(missRecordSize, missRecord, m_missBuffer, m_missUploadBuffer);
   
   dispatchDesc.MissShaderTable.StartAddress = m_missBuffer->GetGPUVirtualAddress();
   dispatchDesc.MissShaderTable.StrideInBytes = missRecordSize;
   dispatchDesc.MissShaderTable.SizeInBytes = missRecordSize;

    void* hitGroupID = stateObjectProps->GetShaderIdentifier(L"MyHitGroup");
    uint8_t hitRecord[D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT] = {};
    memcpy(hitRecord, hitGroupID, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    uint32_t hitGroupRecordSize = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
    CreateUploadBuffer(hitGroupRecordSize, hitRecord, m_hitGroupBuffer, m_hitGroupUploadBuffer);
   
    dispatchDesc.HitGroupTable.StartAddress = m_hitGroupBuffer->GetGPUVirtualAddress();
    dispatchDesc.HitGroupTable.StrideInBytes = hitGroupRecordSize;
    dispatchDesc.HitGroupTable.SizeInBytes = hitGroupRecordSize;
}

void DX12::CreateDepthStencilBuffer(int& width, int& height)
{
    HRESULT hr;
    DXGI_FORMAT depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    CD3DX12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        depthFormat,
        width,
        height,
        1, 1, // array size, mip levels
        1, 0, // sample count, quality
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
    );

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = depthFormat;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&depthStencilBuffer)
    );
    COM_ERROR_IF_FAILED(hr, "Failed to commit resource");

    device->CreateDepthStencilView(
        depthStencilBuffer.Get(),
        nullptr,
        dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );
}

void DX12::InitializeBuffers()
{
    dynamicCB = std::make_unique<DynamicUploadBuffer>(device.Get(), 8 * 1024 * 1024); // 8 MB

    CD3DX12_DESCRIPTOR_RANGE1 srvRangeGbuffer;
    srvRangeGbuffer.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GBUFFER_TEXTURES_NUM, 0, 0); // space0
    CD3DX12_DESCRIPTOR_RANGE1 cubeMapRange;
    cubeMapRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4); // space0 cubeMap
    CD3DX12_DESCRIPTOR_RANGE1 srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 1); // space1
    CD3DX12_DESCRIPTOR_RANGE1 hdrTextRange;
    hdrTextRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 3); // space3
    CD3DX12_DESCRIPTOR_RANGE1 srvStructuredBuffer;
    srvStructuredBuffer.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2); // space2

    CD3DX12_DESCRIPTOR_RANGE1 prefilterRange;
    prefilterRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 4); // t0 space4 prefilterMap
    CD3DX12_DESCRIPTOR_RANGE1 irradianceRange;
    irradianceRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 4); // t1 space4 irradianceMap
    CD3DX12_DESCRIPTOR_RANGE1 brdfRange;
    brdfRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 4); // t2 space4 brdfMap
    CD3DX12_DESCRIPTOR_RANGE1 raytracingUAVRange;
    raytracingUAVRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 5); // u0 space5 raytarcing UAV output
    CD3DX12_DESCRIPTOR_RANGE1 raytracingSrvRange;
    raytracingSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 6); // space6 raytracing srv
    CD3DX12_DESCRIPTOR_RANGE1 raytracingLightPassSrvRange;
    raytracingLightPassSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 4); // t3 space4 raytracing lightpass input

    CD3DX12_ROOT_PARAMETER1 rootParams[19];
    rootParams[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE , D3D12_SHADER_VISIBILITY_VERTEX); // b0: VS transform matrices
    rootParams[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);  // b0: PS
    rootParams[2].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL); // t1 space1: PS textures
    rootParams[3].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX); // b1 VS : skinning data
    rootParams[4].InitAsDescriptorTable(1, &srvRangeGbuffer, D3D12_SHADER_VISIBILITY_PIXEL); // t0 space0: PS Gbuffer textures
    rootParams[5].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);  // b1: PS Material
    rootParams[6].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);  // b2: PS Camera
    rootParams[7].InitAsDescriptorTable(1, &srvStructuredBuffer, D3D12_SHADER_VISIBILITY_ALL); // t0 space2: light's structure buffer in
    rootParams[8].InitAsDescriptorTable(1, &hdrTextRange, D3D12_SHADER_VISIBILITY_PIXEL); // t0 space3: PS cubemap textures
    rootParams[9].InitAsDescriptorTable(1, &cubeMapRange, D3D12_SHADER_VISIBILITY_PIXEL); // t0 space0: PS Gbuffer cubeMap texture
    rootParams[10].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);  // b3: PS PBR buffer

    rootParams[11].InitAsDescriptorTable(1, &prefilterRange, D3D12_SHADER_VISIBILITY_PIXEL); // t0 space4: PS prefilter map
    rootParams[12].InitAsDescriptorTable(1, &irradianceRange, D3D12_SHADER_VISIBILITY_PIXEL); // t1 space4: PS irradiance map
    rootParams[13].InitAsDescriptorTable(1, &brdfRange, D3D12_SHADER_VISIBILITY_PIXEL); // t2 space4: PS brdf map

    rootParams[14].InitAsConstantBufferView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);  // b4: PS lights' data

    rootParams[15].InitAsShaderResourceView(0, 5, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL); // t0 space 5: ray tracing TLAS buffer
    rootParams[16].InitAsDescriptorTable(1, &raytracingUAVRange, D3D12_SHADER_VISIBILITY_ALL); // u0 space5: UAV raytracing UAV output
    rootParams[17].InitAsDescriptorTable(1, &raytracingSrvRange, D3D12_SHADER_VISIBILITY_PIXEL); // t0 space6: PS raytracing map
    rootParams[18].InitAsDescriptorTable(1, &raytracingLightPassSrvRange, D3D12_SHADER_VISIBILITY_PIXEL); // t0 space6: PS raytracing map

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init_1_1(_countof(rootParams), rootParams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    CreateRootSignature(rootSigDesc);
}

// Transition back buffer to render target
void DX12::TransitionBackBufferToRTV()
{
    m_barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    commandList->ResourceBarrier(1, &m_barrier);
}

// Transition back buffer to present
void DX12::TransitionBackBufferToPresent()
{
    m_barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    commandList->ResourceBarrier(1, &m_barrier);
}

void DX12::SetRenderTargetToBackBuffer()
{
    // Set render target
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
    // Set depth-stencil
    dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    // Clear render target to a color
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(
        dsvHandle,
        D3D12_CLEAR_FLAG_DEPTH,
        1.0f,
        0,
        0,
        nullptr
    );
}
void DX12::StartRenderFrame(ECS::SceneManager* sceneManager,GFXGui& gui, Camera& camera, int width, int height, float& dt)
{
    // Reset allocator and command list
    ResetCommands();
    dynamicCB->Reset();

    TransitionBackBufferToRTV();
    ID3D12DescriptorHeap* heaps[] = { sharedSrvHeap.Get() };
    commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetPipelineState(pipelineState.Get());

    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, width, height };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
 

    SetRenderTargetToBackBuffer();
}

void DX12::EndRenderFrame(ECS::SceneManager* sceneManager, GFXGui& gui, Camera& camera, int width, int height, float& dt)
{

    gui.EndRender(commandList.Get());

    TransitionBackBufferToPresent();
    SubmitCommand();
    // Present the frame
    swapChain->Present(1, 0);
    //Update frame index
    frameIndex = swapChain->GetCurrentBackBufferIndex();
}
