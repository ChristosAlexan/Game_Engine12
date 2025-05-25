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

    const wchar_t* args[] = {
        filename.c_str(), // For error reporting
        L"-E", entryPoint.c_str(),
        L"-T", targetProfile.c_str(),
        L"-Zpr",             // Row-major matrices
        L"-Zi",              // Debug info
        L"-Qembed_debug",    // Embed debug info
        L"-Od"               // No optimizations for debugging
    };

    Microsoft::WRL::ComPtr<IDxcResult> result;
    compiler->Compile(
        &sourceBuffer,
        args, _countof(args),
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