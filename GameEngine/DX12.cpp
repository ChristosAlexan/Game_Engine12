
#define INITGUID
#include <d3d12.h>
#include "DX12.h"
#include <stdexcept>
#include "ErrorLogger.h"
#include "GFX_MACROS.h"

// Pretty-print a state object tree.
inline void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc)
{
    std::wstringstream wstr;
    wstr << L"\n";
    wstr << L"--------------------------------------------------------------------\n";
    wstr << L"| D3D12 State Object 0x" << static_cast<const void*>(desc) << L": ";
    if (desc->Type == D3D12_STATE_OBJECT_TYPE_COLLECTION) wstr << L"Collection\n";
    if (desc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE) wstr << L"Raytracing Pipeline\n";

    auto ExportTree = [](UINT depth, UINT numExports, const D3D12_EXPORT_DESC* exports)
        {
            std::wostringstream woss;
            for (UINT i = 0; i < numExports; i++)
            {
                woss << L"|";
                if (depth > 0)
                {
                    for (UINT j = 0; j < 2 * depth - 1; j++) woss << L" ";
                }
                woss << L" [" << i << L"]: ";
                if (exports[i].ExportToRename) woss << exports[i].ExportToRename << L" --> ";
                woss << exports[i].Name << L"\n";
            }
            return woss.str();
        };

    for (UINT i = 0; i < desc->NumSubobjects; i++)
    {
        wstr << L"| [" << i << L"]: ";
        switch (desc->pSubobjects[i].Type)
        {
        case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
            wstr << L"Global Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
            wstr << L"Local Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
            wstr << L"Node Mask: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << *static_cast<const UINT*>(desc->pSubobjects[i].pDesc) << std::setw(0) << std::dec << L"\n";
            break;
        case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
        {
            wstr << L"DXIL Library 0x";
            auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << lib->DXILLibrary.pShaderBytecode << L", " << lib->DXILLibrary.BytecodeLength << L" bytes\n";
            wstr << ExportTree(1, lib->NumExports, lib->pExports);
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION:
        {
            wstr << L"Existing Library 0x";
            auto collection = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << collection->pExistingCollection << L"\n";
            wstr << ExportTree(1, collection->NumExports, collection->pExports);
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
        {
            wstr << L"Subobject to Exports Association (Subobject [";
            auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
            UINT index = static_cast<UINT>(association->pSubobjectToAssociate - desc->pSubobjects);
            wstr << index << L"])\n";
            for (UINT j = 0; j < association->NumExports; j++)
            {
                wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
            }
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
        {
            wstr << L"DXIL Subobjects to Exports Association (";
            auto association = static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
            wstr << association->SubobjectToAssociate << L")\n";
            for (UINT j = 0; j < association->NumExports; j++)
            {
                wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
            }
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
        {
            wstr << L"Raytracing Shader Config\n";
            auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(desc->pSubobjects[i].pDesc);
            wstr << L"|  [0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << L" bytes\n";
            wstr << L"|  [1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << L" bytes\n";
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
        {
            wstr << L"Raytracing Pipeline Config\n";
            auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(desc->pSubobjects[i].pDesc);
            wstr << L"|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
            break;
        }
        case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
        {
            wstr << L"Hit Group (";
            auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC*>(desc->pSubobjects[i].pDesc);
            wstr << (hitGroup->HitGroupExport ? hitGroup->HitGroupExport : L"[none]") << L")\n";
            wstr << L"|  [0]: Any Hit Import: " << (hitGroup->AnyHitShaderImport ? hitGroup->AnyHitShaderImport : L"[none]") << L"\n";
            wstr << L"|  [1]: Closest Hit Import: " << (hitGroup->ClosestHitShaderImport ? hitGroup->ClosestHitShaderImport : L"[none]") << L"\n";
            wstr << L"|  [2]: Intersection Import: " << (hitGroup->IntersectionShaderImport ? hitGroup->IntersectionShaderImport : L"[none]") << L"\n";
            break;
        }
        }
        wstr << L"|--------------------------------------------------------------------\n";
    }
    wstr << L"\n";
    OutputDebugStringW(wstr.str().c_str());
}

DX12::DX12()
{
    timer.Start();
}

DX12::~DX12()
{
    // Wait for the gpu to release the resources
    WaitForGPU(commandQueue.Get(), fence.Get(), fenceEvent, fenceValue);
    m_sbtUploadBuffer.Reset();
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

ID3D12RootSignature* DX12::GetRasterRootSignature() const
{
    return m_rasterRootSignature.Get();
}

ID3D12RootSignature* DX12::GetGlobalRaytracingRootSignature() const
{
    return m_globalRaytracingRootSignature.Get();
}

ID3D12RootSignature* DX12::GetLocalRaytracingRootSignature() const
{
    return m_localRaytracingRootSignature.Get();
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

void DX12::CreateRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc, Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature)
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
    psoDesc.pRootSignature = m_rasterRootSignature.Get();
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

void DX12::CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Local root signature to be used in a ray gen shader.
    {
        auto* localRootSubobject = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        //auto localRootSignature = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        assert(m_localRaytracingRootSignature != nullptr && "Local root signature is null!");
        localRootSubobject->SetRootSignature(m_localRaytracingRootSignature.Get());
        // Shader association
        auto* rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSubobject);
        rootSignatureAssociation->AddExport(c_raygenShaderName);
    }
}

void DX12::CreateRTPSO(IDxcBlob* rayTracingBlob)
{

    D3D12_DXIL_LIBRARY_DESC dxilLibDesc = {};
    dxilLibDesc.DXILLibrary.BytecodeLength = rayTracingBlob->GetBufferSize();
    dxilLibDesc.DXILLibrary.pShaderBytecode = rayTracingBlob->GetBufferPointer();
    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(dxilLibDesc.DXILLibrary.pShaderBytecode, dxilLibDesc.DXILLibrary.BytecodeLength);
    lib->SetDXILLibrary(&libdxil);
    lib->DefineExport(c_raygenShaderName);
    lib->DefineExport(c_closestHitShaderName);
    lib->DefineExport(c_missShaderName);

    auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(c_closestHitShaderName);
    hitGroup->SetHitGroupExport(c_hitGroupName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = 4 * sizeof(float);   // float4 color
    UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
    shaderConfig->Config(payloadSize, attributeSize);

    //CreateLocalRootSignatureSubobjects(&raytracingPipeline);

    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(m_globalRaytracingRootSignature.Get());

    auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    UINT maxRecursionDepth = 1; // primary rays only. 
    pipelineConfig->Config(maxRecursionDepth);

#if _DEBUG
    PrintStateObjectDesc(raytracingPipeline);
#endif

    HRESULT hr = device->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&rtpso));
    COM_ERROR_IF_FAILED(hr, "Failed to create raytracing state object!");
}

void DX12::CreateSBT(UINT numHitGroups)
{
    m_sbtBuffer.Reset();
    m_sbtUploadBuffer.Reset();

    const UINT shaderIdSize = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
    const UINT recordSize = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
    const UINT alignedRecordSize = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

    //UINT numHitGroups = 2;
    UINT sbtSize = alignedRecordSize * (1 + 1 + numHitGroups); // RayGen + Miss + HitGroup(s)

    // --- Create the DEFAULT (GPU) SBT buffer ---
    CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC sbtBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sbtSize, D3D12_RESOURCE_FLAG_NONE);
    HRESULT hr = device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &sbtBufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_sbtBuffer)
    );
    COM_ERROR_IF_FAILED(hr, "failed to create SBT default heap commited resource!");

    // --- Create the UPLOAD buffer ---
    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    hr = device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &sbtBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_sbtUploadBuffer)
    );
    COM_ERROR_IF_FAILED(hr, "failed to create SBT upload heap commited resource!");

    uint8_t* pData = nullptr;
    m_sbtUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pData));

    Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> stateObjectProps;
    rtpso.As(&stateObjectProps);

    // --- RayGen
    void* raygenID = stateObjectProps->GetShaderIdentifier(c_raygenShaderName);
    memcpy(pData, raygenID, shaderIdSize);

    // --- Miss
    void* missID = stateObjectProps->GetShaderIdentifier(c_missShaderName);
    memcpy(pData + alignedRecordSize, missID, shaderIdSize);

    // --- HitGroup(s)
    for (UINT i = 0; i < numHitGroups; ++i)
    {
        void* hitID = stateObjectProps->GetShaderIdentifier(c_hitGroupName);
        if (!hitID) std::cerr << "Hit group ID is null!\n";
        memcpy(pData + alignedRecordSize * (2 + i), hitID, shaderIdSize);
    }
    m_sbtUploadBuffer->Unmap(0, nullptr);

    // --- Copy SBT upload buffer to GPU buffer
    commandList->CopyBufferRegion(m_sbtBuffer.Get(), 0, m_sbtUploadBuffer.Get(), 0, sbtSize);

    dispatchDesc = D3D12_DISPATCH_RAYS_DESC();
    dispatchDesc.Width = GetScreenWidth();
    dispatchDesc.Height = GetScreenHeight();
    dispatchDesc.Depth = 1;

    dispatchDesc.RayGenerationShaderRecord.StartAddress = m_sbtUploadBuffer->GetGPUVirtualAddress();
    dispatchDesc.RayGenerationShaderRecord.SizeInBytes = recordSize;

    dispatchDesc.MissShaderTable.StartAddress = m_sbtUploadBuffer->GetGPUVirtualAddress() + alignedRecordSize;
    dispatchDesc.MissShaderTable.StrideInBytes = recordSize;
    dispatchDesc.MissShaderTable.SizeInBytes = recordSize;

    dispatchDesc.HitGroupTable.StartAddress = m_sbtUploadBuffer->GetGPUVirtualAddress() + 2 * alignedRecordSize;
    dispatchDesc.HitGroupTable.StrideInBytes = recordSize;
    dispatchDesc.HitGroupTable.SizeInBytes = numHitGroups * recordSize;
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
    rootParams[4].InitAsDescriptorTable(1, &srvRangeGbuffer, D3D12_SHADER_VISIBILITY_ALL); // t0 space0: PS Gbuffer textures
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
    CreateRootSignature(rootSigDesc, m_rasterRootSignature);



    CD3DX12_ROOT_PARAMETER1  globalRaytracingRootParams[4];
    globalRaytracingRootParams[0].InitAsDescriptorTable(1, &srvRangeGbuffer, D3D12_SHADER_VISIBILITY_ALL); // t0 space0: PS Gbuffer textures
    globalRaytracingRootParams[1].InitAsShaderResourceView(0, 6, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL); // t0 space 5: ray tracing TLAS buffer
    globalRaytracingRootParams[2].InitAsDescriptorTable(1, &raytracingUAVRange, D3D12_SHADER_VISIBILITY_ALL); // u0 space5: UAV raytracing UAV output
    globalRaytracingRootParams[3].InitAsDescriptorTable(1, &srvStructuredBuffer, D3D12_SHADER_VISIBILITY_ALL); // t0 space2: light's structure buffer in
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC globalRaytracingRootSigDesc;
    globalRaytracingRootSigDesc.Init_1_1(_countof(globalRaytracingRootParams), globalRaytracingRootParams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_NONE);
    CreateRootSignature(globalRaytracingRootSigDesc, m_globalRaytracingRootSignature);

    /*CD3DX12_ROOT_PARAMETER1  localRaytracingRootParams[1];
    localRaytracingRootParams[0].InitAsDescriptorTable(1, &srvRangeGbuffer, D3D12_SHADER_VISIBILITY_ALL); // t0 space0: PS Gbuffer textures
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC localRaytracingRootSigDesc;
    localRaytracingRootSigDesc.Init_1_1(_countof(localRaytracingRootParams), localRaytracingRootParams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
    CreateRootSignature(localRaytracingRootSigDesc, m_localRaytracingRootSignature);*/
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

    commandList->SetGraphicsRootSignature(GetRasterRootSignature());
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
