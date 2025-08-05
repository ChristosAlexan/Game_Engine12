#include "DXCShaderCompiler.h"
#include <dxcapi.h>
#include <stdexcept>

DXCShaderCompiler::DXCShaderCompiler()
{
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    utils->CreateDefaultIncludeHandler(&includeHandler);
}

Microsoft::WRL::ComPtr<IDxcBlob> DXCShaderCompiler::CompileShader(
    const std::wstring& filename,
    const std::wstring& entryPoint,
    const std::wstring& targetProfile)
{
    HRESULT hr;
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
    hr = utils->LoadFile(filename.c_str(), nullptr, &sourceBlob);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to load shader!");
    }
    DxcBuffer sourceBuffer = {};
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();
    sourceBuffer.Encoding = DXC_CP_ACP;

    std::vector<LPCWSTR> args;
    args.push_back(filename.c_str());
    if (!entryPoint.empty()) 
    {
        args.push_back(L"-E");
        args.push_back(entryPoint.c_str());
    }
    args.push_back(L"-T");
    args.push_back(targetProfile.c_str());
    args.push_back(L"-Zpr"); // Row-major matrices
    args.push_back(L"-Zi"); // Debug info
    args.push_back(L"-Qembed_debug"); // Embed debug info in blob
#ifdef _DEBUG
    args.push_back(L"-Od"); // Disable optimizations
#else
    args.push_back(L"-O3"); // Enable optimizations
#endif
   
    Microsoft::WRL::ComPtr<IDxcResult> result;
    compiler->Compile(
        &sourceBuffer,
        args.data(), args.size(),
        includeHandler.Get(),
        IID_PPV_ARGS(&result)
    );

    Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    if (errors && errors->GetStringLength() > 0) {
        OutputDebugStringA(errors->GetStringPointer());
    }

    Microsoft::WRL::ComPtr<IDxcBlob> compiledBlob;
    hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledBlob), nullptr);
    if (FAILED(hr)) {
        throw std::runtime_error("Shader compilation failed");
    }

    return compiledBlob;
}