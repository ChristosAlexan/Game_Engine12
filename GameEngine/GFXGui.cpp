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

void GFXGui::SelectEntity(ECS::SceneManager* sceneManager, UINT screenWidth, UINT screenHeight, Camera& camera)
{
	float closestT = FLT_MAX;
	auto scene = sceneManager->GetCurrentScene();
	auto world = scene->GetWorld();
	for (const auto& [entityID, transformComponent] : world->GetAllTransformComponents())
	{
		auto trans = world->GetComponent<ECS::TransformComponent>(entityID);

		Ray ray = RaycastPicking(screenWidth, screenHeight, camera);

		if (IntersectsAABB(ray, scene->GetWorldAABB(trans), m_hitT) && (m_hitT < closestT))
		{
			closestT = m_hitT;
			m_closestEntityID = entityID;
			m_closestTransform = trans;
		}
	}

}

void GFXGui::UpdateTransformUI(ECS::SceneManager* sceneManager, UINT screenWidth, UINT screenHeight, Camera& camera)
{
	ImGui::Begin("Engine");

	std::string entityLabel = "Entity" + std::to_string(m_closestEntityID) + "##" + std::to_string(m_closestEntityID);

	if (ImGui::CollapsingHeader(entityLabel.c_str()))
	{
		std::string posOffset = "Pos##" + std::to_string(m_closestEntityID);
		std::string scaleOffset = "Scale##" + std::to_string(m_closestEntityID);
		std::string rotOffset = "Rot##" + std::to_string(m_closestEntityID);

		ImGui::DragFloat3(posOffset.c_str(), &m_closestTransform->position.x, 0.05f);
		ImGui::DragFloat3(rotOffset.c_str(), &m_closestTransform->rotation.x, 0.05f);
		ImGui::DragFloat3(scaleOffset.c_str(), &m_closestTransform->scale.x, 0.05f);
	}
	ImGui::End();
}

void GFXGui::BeginRender()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();

	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();
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


bool GFXGui::IntersectsAABB(const Ray& ray, const ECS::AABB& box, float& outDistance)
{
	using namespace DirectX;

	float tMin = 0.0f;
	float tMax = FLT_MAX;

	for (int i = 0; i < 3; ++i)
	{
		float rayOrigin = XMVectorGetByIndex(ray.origin, i);
		float rayDir = XMVectorGetByIndex(ray.direction, i);
		float boxMin = XMVectorGetByIndex(box.min, i);
		float boxMax = XMVectorGetByIndex(box.max, i);

		if (fabs(rayDir) < 1e-8f)
		{
			if (rayOrigin < boxMin || rayOrigin > boxMax)
				return false;
		}
		else
		{
			float ood = 1.0f / rayDir;
			float t1 = (boxMin - rayOrigin) * ood;
			float t2 = (boxMax - rayOrigin) * ood;

			if (t1 > t2) std::swap(t1, t2);

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
				return false;
		}
	}

	// Only accept forward-facing hits
	if (tMin < 0.0f)
		return false;

	outDistance = tMin;
	return true;
}

GFXGui::Ray GFXGui::RaycastPicking(UINT screenWidth, UINT screenHeight, Camera& camera)
{
	ImVec2 mousePos = ImGui::GetMousePos();
	float ndcX = (2.0f * mousePos.x) / screenWidth - 1.0f;
	float ndcY = 1.0f - (2.0f * mousePos.y) / screenHeight; // Invert Y for DX

	DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(nullptr, camera.GetViewMatrix());
	DirectX::XMMATRIX invProj = DirectX::XMMatrixInverse(nullptr, camera.GetProjectionMatrix());

	DirectX::XMVECTOR rayClip = DirectX::XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);
	DirectX::XMVECTOR rayEye = XMVector3TransformCoord(rayClip, invProj);
	//rayEye = DirectX::XMVectorSetZ(rayEye, 1.0f); // Forward direction

	DirectX::XMVECTOR rayDir = DirectX::XMVector3Normalize(XMVector3TransformNormal(rayEye, invView));
	DirectX::XMVECTOR rayOrigin = camera.GetPositionVector();

	return { rayOrigin, rayDir };
}
