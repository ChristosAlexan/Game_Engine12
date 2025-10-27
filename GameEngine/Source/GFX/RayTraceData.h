#pragma once
#include "DX12Includes.h"

namespace ECS
{
	struct rayTracingResources
	{
		Microsoft::WRL::ComPtr<ID3D12StateObject> rtpso;
		std::string shaderFile;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_sbtBuffer, m_sbtUploadBuffer;
	};

	struct BLAS
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> result;  // final BLAS buffer
		Microsoft::WRL::ComPtr<ID3D12Resource> scratch; // temp buffer
		D3D12_RAYTRACING_GEOMETRY_DESC geometry = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild{};
		bool allowUpdate = false;
	};

	struct TLAS
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> tlasBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> scratchBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer;
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild{};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuildDesc{};
		bool bInit = false;
	};
}

