#pragma once
#include "RayTraceData.h"
#include <entt/entt.hpp>
#include "DX12.h"

namespace ECS
{
	class Scene;
}

class TLASBuilder
{
public:
	TLASBuilder();
	void Build(ECS::Scene* scene);
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& GetInstances();
private:
	// Build ray tracing accelaration structure
	void BuildRAS(DX12& dx12);
public:
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> m_instanceDescs;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_tlasBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_scratchBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_instanceBuffer;
};