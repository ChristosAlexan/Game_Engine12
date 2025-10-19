#pragma once
#include "DX12.h"
#include <PhysX/PxPhysicsAPI.h>
#include "MeshData.h"
#include "Camera.h"

namespace PHYSICS
{
	class PhysicsDebugDraw
	{
	public:
		PhysicsDebugDraw(physx::PxScene* aScene);
		void DebugDraw(DX12& dx12, Camera& camera);

	private:
		physx::PxScene* m_aScene = nullptr;
		ECS::MeshData m_mesh;
		ECS::GpuMesh m_gpuMesh;
	};
	
}


