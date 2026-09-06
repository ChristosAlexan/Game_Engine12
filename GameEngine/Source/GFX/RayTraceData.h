#pragma once
#include "DX12Includes.h"
#include <cstdint>
#include "StructuredBuffer.h"

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

	struct RTVertexData
	{
		DirectX::XMFLOAT3 position;
		float padding1 = 0.0f;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT2 padding2 = DirectX::XMFLOAT2(0.0f, 0.0f);
	};

	struct RTIndexData
	{
		uint32_t indices;
		DirectX::XMFLOAT3 padding = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	};

	struct RTEntityHandle
	{
		StructuredBuffer<RTVertexData> rtVertexGpuData;
		StructuredBuffer<RTIndexData> rtIndexGpuData;
		StructuredBuffer<struct MeshDataOffsets> rtMeshDataOffsets;

		D3D12_CPU_DESCRIPTOR_HANDLE cpuVertexHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuVertexHandle{};
		D3D12_CPU_DESCRIPTOR_HANDLE cpuIndexHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuIndexHandle{};
		D3D12_CPU_DESCRIPTOR_HANDLE cpuOffsetsHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuOffsetsHandle{};
	};

}

