#include "PhysicsDebugDraw.h"
#include "../MeshData.h"

namespace PHYSICS
{
	PhysicsDebugDraw::PhysicsDebugDraw(physx::PxScene* aScene)
	{
		m_aScene = aScene;
	}

	void PhysicsDebugDraw::DebugDraw(DX12& dx12, Camera& camera)
	{
		m_gpuMesh.cpuMesh.reset();
		m_mesh.vertices.clear();

		ID3D12DescriptorHeap* heaps[] = { dx12.GetSharedSrvHeap() };
		dx12.GetCmdList()->SetDescriptorHeaps(1, heaps);
		dx12.GetCmdList()->SetPipelineState(dx12.pipelineState_debug.Get());

		CB_VS_SimpleShader vsCB = {};
		CB_VS_AnimationShader skinningCB = {};
		CB_PS_Material psMaterialCB = {};

		vsCB.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
		vsCB.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());
		vsCB.worldMatrix = DirectX::XMMatrixIdentity();

		psMaterialCB.color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		psMaterialCB.hasTextures = false;
		psMaterialCB.metalness = 0;
		psMaterialCB.roughness = 0;
		psMaterialCB.useAlbedo = false;
		psMaterialCB.useNormals = false;
		psMaterialCB.useRoughnessMetal = false;
		psMaterialCB.padding = 0.0f;

		skinningCB.HasAnim = false;
		if (dx12.GetCmdList())
		{
			if (dx12.dynamicCB)
			{
				dx12.GetCmdList()->SetGraphicsRootConstantBufferView(0, dx12.dynamicCB->Allocate(vsCB));
				dx12.GetCmdList()->SetGraphicsRootConstantBufferView(3, dx12.dynamicCB->Allocate(skinningCB));
				dx12.GetCmdList()->SetGraphicsRootConstantBufferView(5, dx12.dynamicCB->Allocate(psMaterialCB));
			}
		}
		const physx::PxRenderBuffer& rb = m_aScene->getRenderBuffer();
		const physx::PxDebugLine* line = rb.getLines();

		for (physx::PxU32 i = 0; i < rb.getNbLines(); ++i)
		{
			DirectX::XMFLOAT3 p0{ line[i].pos0.x, line[i].pos0.y, line[i].pos0.z };
			DirectX::XMFLOAT3 p1{ line[i].pos1.x, line[i].pos1.y, line[i].pos1.z };

			Vertex vertex0;
			Vertex vertex1;
		
			vertex0.pos = p0;
			vertex1.pos = p1;

			m_mesh.vertices.push_back(vertex0);
			m_mesh.vertices.push_back(vertex1);
		}

		if (m_mesh.vertices.size() > 0)
		{
			dx12.GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
			m_gpuMesh.cpuMesh = std::make_shared<ECS::MeshData>(m_mesh);
			m_gpuMesh.Upload(dx12.GetDevice(), dx12.GetCmdList());
			m_gpuMesh.Draw(dx12.GetCmdList());
		}
	}
}

