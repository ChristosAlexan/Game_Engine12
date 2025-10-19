#include "ComputeSkinning.h"
#include "Scene.h"
#include "RenderingManager.h"

ComputeSkinning::ComputeSkinning()
{
}

void ComputeSkinning::Initialize()
{
	unsigned int threadsPerGroup[3] = { 256, 1, 1 };
	ComputeWrapper::Initialize(threadsPerGroup);
}

void ComputeSkinning::Compute(ECS::Scene* scene)
{
	ComputeWrapper::Compute(scene);

	auto& dx12 = scene->GetRenderingManager()->GetDX12();

	CB_CS_AnimationShader skinningCB = {};

	auto group = scene->GetRegistry().group<>(entt::get<ECS::RenderComponent, AnimatorComponent>);

	for (auto entity : group)
	{
		auto& renderComponent = group.get<ECS::RenderComponent>(entity);
		auto& gpuMesh = group.get<ECS::RenderComponent>(entity).mesh;
		auto& cpuMesh = group.get<ECS::RenderComponent>(entity).mesh->cpuMesh;

		if (renderComponent.meshType == ECS::SKELETAL_MESH)
		{
			auto& animatorComponent = group.get<AnimatorComponent>(entity);

			ID3D12DescriptorHeap* heaps[] = { dx12.GetSharedSrvHeap() };
			dx12.GetCmdList()->SetDescriptorHeaps(1, heaps);
			dx12.GetCmdList()->SetComputeRootSignature(dx12.GetComputeRootSignature());


			if (!animatorComponent.finalTransforms.empty())
			{
				size_t matrixCount = animatorComponent.finalTransforms.size();
				if(matrixCount <= ARRAYSIZE(skinningCB.skinningMatrix))
					memcpy(skinningCB.skinningMatrix, animatorComponent.finalTransforms.data(), matrixCount * sizeof(DirectX::XMFLOAT4X4));
			}

			if (dx12.GetCmdList())
			{
				// Transition back to unorder access
				renderComponent.skinningOutData.skinningVertexBufferFinalTransform.GetResource()->TransitionState(dx12.GetCmdList(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

				dx12.GetCmdList()->SetPipelineState(dx12.pipelineState_compute.Get());
				dx12.GetCmdList()->SetComputeRootDescriptorTable(
					0, // Root parameter skinning structured buffer input
					gpuMesh->skinningGpuHandleIn
				);

				std::size_t vertexCount = cpuMesh->vertices.size();
				skinningCB.vertexCount = vertexCount;

				dx12.GetCmdList()->SetComputeRootConstantBufferView(1, dx12.dynamicCB->Allocate(skinningCB));

				dx12.GetCmdList()->SetComputeRootDescriptorTable(
					2, // Root parameter skinning structured buffer output
					renderComponent.skinningOutData.skinningGpuUavHandleFinalTransform
				);

				UINT group = (vertexCount + m_threadsPerGroup[0] - 1) / m_threadsPerGroup[0];

				dx12.GetCmdList()->Dispatch(group, 1, 1);

				// Transition to shader resource
				renderComponent.skinningOutData.skinningVertexBufferFinalTransform.GetResource()->TransitionState(dx12.GetCmdList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			}
		}
	}
}
