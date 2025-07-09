
#define INITGUID
#include <d3d12.h>
#include "DX12.h"
#include <stdexcept>
#include "ErrorLogger.h"
#include "DX12_GLOBALS.h"

std::unique_ptr<DescriptorAllocator> g_descAllocator;

DX12::DX12()
{
    timer.Start();
}


void DX12::ResetCommandAllocator()
{
    // Reset command list before issuing GPU uploads
    commandAllocator->Reset();
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
    commandQueue->Signal(fence.Get(), fenceValue);
    fenceValue++;

    if (fence->GetCompletedValue() < currentFence)
    {
        fence->SetEventOnCompletion(currentFence, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }

}

void DX12::InitDescAllocator(ID3D12DescriptorHeap* heap)
{
    g_descAllocator = std::make_unique<DescriptorAllocator>(device.Get(), heap, 512, true);
}

ID3D12CommandQueue* DX12::GetCommandQueue() const
{
    return commandQueue.Get();
}

ID3D12Device* DX12::GetDevice() const
{
    return device.Get();
}

ID3D12GraphicsCommandList* DX12::GetCmdList() const
{
    return commandList.Get();
}

ID3D12DescriptorHeap* DX12::GetDescriptorHeap() const
{
    return rtvHeap.Get();
}

void DX12::Initialize(HWND hwnd, int& width, int& height)
{
    CreateDeviceAndFactory();
    CreateCommandObjects();
    CreateSwapChainAndRTVs(hwnd, width, height);
    CreateFenceAndSyncObjects();
    CreateDescriptorHeaps();
    CreateDepthStencilBuffer(width, height);
    CreateSamplerStates();
    InitializeConstantBuffers();
    InitializeShaders();
    InitDescAllocator(sharedHeap.Get());

    m_renderTexture.Initialize(device.Get(), width, height);
}

void DX12::CreateDeviceAndFactory()
{
    HRESULT hr;
    UINT dxgiFactoryFlags = 0;
  

#if defined(_DEBUG)
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

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue; // skip software adapters
        }

        hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
            break;
    }
    COM_ERROR_IF_FAILED(hr, "Failed to create device");
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
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
    samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(
        0,                                // shaderRegister (s0)
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP  // addressW
    );
}

void DX12::CreateRootSignature(CD3DX12_ROOT_SIGNATURE_DESC& rootSigDesc)
{
    // Serialize
    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    HRESULT hr = D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
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
    heapDesc.NumDescriptors = 256;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&sharedHeap));
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
    {
        DXCShaderCompiler compiler;

        auto vsBlob = compiler.CompileShader(L"VertexShader12.hlsl", L"Main", L"vs_6_7");
        auto psBlob = compiler.CompileShader(L"PixelShader12.hlsl", L"Main", L"ps_6_7");

        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0  },
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0  },
            {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0  },
            { "BONEWEIGHTS",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "BONEINDICES",   0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        UINT layoutSize = _countof(inputLayout);
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState, inputLayout, layoutSize);
    }

    {
        DXCShaderCompiler compiler;

        auto vsBlob = compiler.CompileShader(L"VertexShader_2D.hlsl", L"Main", L"vs_6_7");
        auto psBlob = compiler.CompileShader(L"PixelShader_2D.hlsl", L"Main", L"ps_6_7");

        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        UINT layoutSize = _countof(inputLayout);
        CreatePSO(vsBlob.Get(), psBlob.Get(), pipelineState_2D, inputLayout, layoutSize);
    }
  
}

void DX12::CreatePSO(IDxcBlob* vsBlob, IDxcBlob* psBlob, Microsoft::WRL::ComPtr<ID3D12PipelineState>& PSO_pipeline, D3D12_INPUT_ELEMENT_DESC* inputLayout, UINT size)
{
    HRESULT hr;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, size };
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO_pipeline));
    COM_ERROR_IF_FAILED(hr, "Failed to create pipeline state");
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

void DX12::InitializeConstantBuffers()
{
    dynamicCB = std::make_unique<DynamicUploadBuffer>(device.Get(), 8 * 1024 * 1024); // 8 MB

    CD3DX12_DESCRIPTOR_RANGE srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);

    CD3DX12_ROOT_PARAMETER rootParams[4];
    rootParams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b0 VS
    rootParams[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);  // b0 PS
    rootParams[2].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParams[3].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init(_countof(rootParams), rootParams, 1, &samplerDesc,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CreateRootSignature(rootSigDesc);
}

// Transition back buffer to render target
void DX12::TransitionBackBuffer()
{
    m_barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    commandList->ResourceBarrier(1, &m_barrier);
}

void DX12::StartRenderFrame(ECS::SceneManager* sceneManager,GFXGui& gui, Camera& camera, int width, int height, float& dt)
{
    // Reset allocator and command list
    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), nullptr);
    dynamicCB->Reset();
    m_renderTexture.Reset(commandList.Get());

    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetPipelineState(pipelineState.Get());

    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, width, height };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
 
    TransitionBackBuffer();

    // Set render target
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
    // Set depth-stencil
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    // Clear render target to a color
    const float clearColor[] = { 0.1f, 0.1f, 0.3f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(
        dsvHandle,
        D3D12_CLEAR_FLAG_DEPTH,
        1.0f,
        0,
        0,
        nullptr
    );


    ID3D12DescriptorHeap* heaps[] = { sharedHeap.Get() };
    commandList->SetDescriptorHeaps(1, heaps);

    m_renderTexture.SetRenderTarget(commandList.Get(), dsvHandle);
}

void DX12::EndRenderFrame(ECS::SceneManager* sceneManager, GFXGui& gui, Camera& camera, int width, int height, float& dt)
{
    m_renderTexture.RenderFullScreenQuad(commandList.Get(), rtvHeap.Get(), dsvHeap.Get(), frameIndex, rtvDescriptorSize, pipelineState_2D.Get());
  
    gui.EndRender(commandList.Get());


    TransitionBackBuffer();
    SubmitCommand();

    // Present the frame
    swapChain->Present(1, 0);
    //Update frame index
    frameIndex = swapChain->GetCurrentBackBufferIndex();
}