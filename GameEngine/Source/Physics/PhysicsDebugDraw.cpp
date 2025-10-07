#include "PhysicsDebugDraw.h"
#include "../MeshData.h"

namespace PHYSICS
{
	PhysicsDebugDraw::PhysicsDebugDraw(physx::PxScene* aScene)
	{
		m_aScene = aScene;
	}

	void PhysicsDebugDraw::DebugDraw(DX12& dx12)
	{
		const physx::PxRenderBuffer& rb = m_aScene->getRenderBuffer();
		const physx::PxDebugLine* line = rb.getLines();

		m_mesh.vertices.clear();
		
		//for (physx::PxU32 i = 0; i < rb.getNbLines(); ++i)
		//{
		//
		//	DirectX::XMFLOAT3 p0{ line[i].pos0.x, line[i].pos0.y, line[i].pos0.z };
		//	DirectX::XMFLOAT3 p1{ line[i].pos1.x, line[i].pos1.y, line[i].pos1.z };
		//
		//	Vertex vertex0;
		//	Vertex vertex1;
		//
		//	vertex0.pos = p0;
		//	vertex1.pos = p0;
		//	m_mesh.vertices.push_back(vertex0);
		//	m_mesh.vertices.push_back(vertex1);
		//
		//	m_gpuMesh.cpuMesh = std::make_shared<ECS::MeshData>(m_mesh);
		//	m_gpuMesh.Upload(dx12.GetDevice(), dx12.GetCmdList());
		//	m_gpuMesh.Draw(dx12.GetCmdList());
		//}
	}
	
}

