#pragma once
#include "DX12Includes.h"
#include <dxcapi.h>
#include <wrl.h>
#include <string>

class DXCShaderCompiler
{
public:
    DXCShaderCompiler();

    Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
        const std::wstring& filename,
        const std::wstring& entryPoint,
        const std::wstring& targetProfile);

private:
    Microsoft::WRL::ComPtr<IDxcUtils> utils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
};