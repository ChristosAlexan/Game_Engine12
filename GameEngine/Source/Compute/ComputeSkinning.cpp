#include "ComputeSkinning.h"
#include "Scene.h"
#include "RenderingManager.h"
#include "AnimationManager.h"

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
    auto* animationManager = scene->GetAnimationManager();
    auto* cmdList = dx12.GetCmdList();

    auto group = scene->GetRegistry().group<>(entt::get<ECS::RenderComponent, AnimatorComponent>);

    auto* sharedRes = animationManager->skinningOutputData.skinningVertexBufferFinalTransform.GetResource();
    sharedRes->TransitionState(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    ID3D12DescriptorHeap* heaps[] = { dx12.GetSharedSrvHeap() };
    cmdList->SetDescriptorHeaps(1, heaps);
    cmdList->SetComputeRootSignature(dx12.GetComputeRootSignature());
    cmdList->SetPipelineState(dx12.pipelineState_compute.Get());

    for (auto entity : group)
    {
        auto& renderComponent = group.get<ECS::RenderComponent>(entity);
        auto& gpuMesh = renderComponent.mesh;
        auto& cpuMesh = renderComponent.mesh->cpuMesh;

        if (renderComponent.meshType != ECS::SKELETAL_MESH)
            continue;

        auto& animatorComponent = group.get<AnimatorComponent>(entity);

        uint32_t vertexCount = static_cast<uint32_t>(cpuMesh->vertices.size());

        CB_CS_AnimationShader skinningCB = {};
        skinningCB.vertexCount = vertexCount;
        skinningCB.outputOffset = renderComponent.skinnedVertexOffset;

        if (!animatorComponent.finalTransforms.empty())
        {
            size_t matrixCount = animatorComponent.finalTransforms.size();
            if (matrixCount <= ARRAYSIZE(skinningCB.skinningMatrix))
                memcpy(skinningCB.skinningMatrix, animatorComponent.finalTransforms.data(),
                    matrixCount * sizeof(DirectX::XMFLOAT4X4));
        }

        cmdList->SetComputeRootDescriptorTable(0, gpuMesh->skinningGpuHandleIn);
        cmdList->SetComputeRootConstantBufferView(1, dx12.dynamicCB->Allocate(skinningCB));
        cmdList->SetComputeRootDescriptorTable(2, animationManager->skinningOutputData.skinningGpuUavHandleFinalTransform);

        UINT dispatchGroups = (vertexCount + m_threadsPerGroup[0] - 1) / m_threadsPerGroup[0];
        cmdList->Dispatch(dispatchGroups, 1, 1);
    }

    sharedRes->TransitionState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}