#pragma once
#include "DX12Includes.h"

namespace ECS
{
	struct BLAS
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> result;  // final BLAS buffer
		Microsoft::WRL::ComPtr<ID3D12Resource> scratch; // temp buffer
	};

	struct TLAS
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> result;
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
	};
}

