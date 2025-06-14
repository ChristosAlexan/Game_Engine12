#include "GFXGui.h"
#include "ErrorLogger.h"
#include "DX12_GLOBALS.h"


GFXGui::GFXGui()
{
}

bool GFXGui::Initialize(HWND hwnd, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12DescriptorHeap* descriptorHeap)
{
	bool result;
	//Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch


	result = ImGui_ImplWin32_Init(hwnd);

	if (!result)
	{
		ErrorLogger::Log("Failed to initialize ImGui: Win32!");
		return false;
	}
	
	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.Device = device;
	init_info.CommandQueue = cmdQueue;
	init_info.NumFramesInFlight = 2;
	init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	init_info.SrvDescriptorHeap = descriptorHeap;
	init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu) {
		auto handle = g_descAllocator->Allocate();
		*out_cpu = handle.cpuHandle;
		*out_gpu = handle.gpuHandle;
		};

	init_info.SrvDescriptorFreeFn = nullptr;

	result = ImGui_ImplDX12_Init(&init_info);

	if (!result)
	{
		ErrorLogger::Log("Failed to initialize ImGui: DX12!");
		return false;
	}
	EditorStyle();

	return true;
}

void GFXGui::BeginRender()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();

	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
}

void GFXGui::EndRender(ID3D12GraphicsCommandList* cmdList)
{
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
}

void GFXGui::EditorStyle()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

	style->WindowRounding = 10.0f;
	style->FrameRounding = 10.0f;

	colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
	colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.50f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1f, 0.1f, 0.1f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.1f, 0.1f, 0.1f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1f, 0.1f, 0.1f, 0.40f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 0.0f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5f, 0.5f, 0.0f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.2f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.5f, 0.5f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.5f, 0.5f, 0.5f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.2f, 0.2f, 0.2f, 0.80f);
	colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_TabActive] = ImVec4(0.5f, 0.5f, 0.5f, 0.60f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.5f, 0.5f, 0.5f, 0.80f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.5f, 0.5f, 0.5f, 0.40f);
}
