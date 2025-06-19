#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <Windows.h>
#include "DescriptorAllocator.h"
#include "SceneManager.h"
#include "ImGuizmo.h"

class GFXGui
{
public:

	GFXGui();

	bool Initialize(HWND hwnd, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12DescriptorHeap* descriptorHeap);
	void SelectEntity(ECS::SceneManager* sceneManager, UINT screenWidth, UINT screenHeight, Camera& camera);
	void UpdateSelectedEntity(ECS::SceneManager* sceneManager, UINT screenWidth, UINT screenHeight, Camera& camera);
	void UpdateAllEntities(ECS::SceneManager* sceneManager, UINT screenWidth, UINT screenHeight, Camera& camera);
	void BeginRender();
	void EndRender(ID3D12GraphicsCommandList* cmdList);

	void EditorStyle();
private:
	float m_hitT;
	ECS::EntityID m_closestEntityID;
	ECS::TransformComponent* m_closestTransform;
};


