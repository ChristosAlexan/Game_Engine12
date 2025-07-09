#pragma once
#include "DX12.h"
#include "Camera.h"
#include "DynamicUploadBuffer.h"
#include "RenderingECS.h"
#include "ModelData.h"
#include "TransformECS.h"
#include "RenderTargetTexture.h"

class GameWindow;
namespace ECS
{
	class Scene;
	class RenderingManager
	{
	public:
		RenderingManager();
		bool Initialize(GameWindow& game_window, int width, int height);
		void InitializeRenderTargets(ID3D12Device* device, int& width, int& height);
		DX12& GetDX12();
		GFXGui& GetGFXGui();
		void ResetRenderTargets(ID3D12GraphicsCommandList* cmdList);
		void SetCurrentRenderTarget(RenderTargetTexture& renderTarget, ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle);
		void RenderToTexture(RenderTargetTexture& renderTarget, ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* rtvHeap, ID3D12DescriptorHeap* dsvHeap, UINT& frameIndex, UINT& rtvDescriptorSize, ID3D12PipelineState* pipelineState);
		void Render(Scene* scene, Camera& camera, DynamicUploadBuffer* dynamicCB, ID3D12GraphicsCommandList* cmdList,
			TransformComponent& transformComponent, RenderComponent& renderComponent, AnimatorComponent& animatorComponent);

	private:
		RenderTargetTexture m_gBufferTexture;
		DX12 m_dx12;
		GFXGui m_gui;
	};
}


