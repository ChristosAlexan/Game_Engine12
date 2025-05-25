#include "GFXGui.h"

GFXGui::GFXGui()
{
}

void GFXGui::Initialize(HWND hwnd)
{

	//Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	io.ConfigFlags |= ImGuiConfigFlags_None;
	//io.ConfigFlags |= ImGuiBackendFlags_PlatformHasViewports;
	//io.ConfigFlags |= ImGuiBackendFlags_RendererHasViewports;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


	ImGui_ImplWin32_Init(hwnd);
	//ImGui_ImplDX11_Init(device, deviceContext);
	ImGui::StyleColorsDark();
}


void GFXGui::BeginRender()
{
#ifndef DX12_API
	ImGui_ImplDX11_NewFrame();
#else

#endif // DX12_API


	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

void GFXGui::EndRender()
{
	ImGui::Render();
	//ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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
