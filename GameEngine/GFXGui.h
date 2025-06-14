#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <Windows.h>
#include "DescriptorAllocator.h"
#include "SceneManager.h"

class GFXGui
{
public:

	GFXGui();

	bool Initialize(HWND hwnd, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12DescriptorHeap* descriptorHeap);
	void UpdateTransformUI(ECS::SceneManager* sceneManager);
	void BeginRender();
	void EndRender(ID3D12GraphicsCommandList* cmdList);

	void EditorStyle();
};

