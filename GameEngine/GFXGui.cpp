#include "GFXGui.h"
#include "ErrorLogger.h"
#include "MathHelpers.h"

GFXGui::GFXGui()
{
	m_closestEntity = entt::null;
	m_hitT = FLT_MAX;
}

bool GFXGui::Initialize(SDL_Window* sdl_window, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12DescriptorHeap* descriptorHeap, DescriptorAllocator* descAllocator)
{
	bool result;
	//Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	result = ImGui_ImplSDL3_InitForD3D(sdl_window);
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

	static auto handle = descAllocator->Allocate();
	init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu) {
		*out_cpu = handle.cpuHandle;
		*out_gpu = handle.gpuHandle;
		};

	init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info,
		D3D12_CPU_DESCRIPTOR_HANDLE cpu,
		D3D12_GPU_DESCRIPTOR_HANDLE gpu)
		{
			
		};
	
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
	auto renderGroup = scene->GetRegistry().group<>(entt::get<ECS::TransformComponent, ECS::RenderComponent>);
	for (auto [entity, transform, renderComponent] : renderGroup.each())
	{
		Ray ray = RaycastPicking(screenWidth, screenHeight, camera);

		if (IntersectsAABB(ray, GetWorldAABB(&transform, &renderComponent), m_hitT) && (m_hitT < closestT))
		{
			closestT = m_hitT;
			m_closestEntity = entity;
			m_closestTransform = &transform;
			m_closestEntityName = renderComponent.name;
		}
	}
}

void GFXGui::UpdateSelectedEntity(ECS::SceneManager* sceneManager, UINT screenWidth, UINT screenHeight, Camera& camera)
{
	ImGui::Begin("Entity");
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::BeginFrame();

	ImGuizmo::SetRect(0, 0, (float)screenWidth, (float)screenHeight);

	if (m_closestEntity != entt::null)
	{
		DirectX::XMMATRIX viewMatrix = camera.GetViewMatrix();
		DirectX::XMMATRIX projMatrix = camera.GetProjectionMatrix();


		std::string entityLabel = m_closestEntityName + ": " + std::to_string(static_cast<uint32_t>(m_closestEntity)) +
			"##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
		std::string entityName = m_closestEntityName + ": " + std::to_string(static_cast<uint32_t>(m_closestEntity));

		ImGui::Text(entityName.c_str());
		static 	bool mode[3] = {false, false, false};
		static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
		if(ImGui::Checkbox("Trans", &mode[0]))
		{
			operation = ImGuizmo::TRANSLATE;
			mode[1] = false;
			mode[2] = false;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Rot", &mode[1]))
		{
			operation = ImGuizmo::ROTATE;
			mode[0] = false;
			mode[2] = false;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Scale", &mode[2]))
		{
			operation = ImGuizmo::SCALE;
			mode[0] = false;
			mode[1] = false;
		}
		std::string posOffset = "Pos##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
		std::string scaleOffset = "Scale##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
		std::string rotOffset = "Rot##" + std::to_string(static_cast<uint32_t>(m_closestEntity));


		ImGui::DragFloat3(posOffset.c_str(), &m_closestTransform->position.x, 0.05f);
		ImGui::DragFloat3(scaleOffset.c_str(), &m_closestTransform->scale.x, 0.05f);
		if (ImGui::DragFloat4(rotOffset.c_str(), &m_closestTransform->rotation.x, 0.01f)) {
			DirectX::XMVECTOR q = DirectX::XMLoadFloat4(&m_closestTransform->rotation);
			q = DirectX::XMQuaternionNormalize(q);
			DirectX::XMStoreFloat4(&m_closestTransform->rotation, q);
		}
		
		auto scene = sceneManager->GetCurrentScene();
		entt::entity entity = static_cast<entt::entity>(m_closestEntity);

		if (scene->GetRegistry().all_of<ECS::RenderComponent>(entity) && scene->GetRegistry().all_of<AnimatorComponent>(entity))
		{
			ECS::RenderComponent& renderComponent = scene->GetRegistry().get<ECS::RenderComponent>(entity);
			AnimatorComponent& animComponent = scene->GetRegistry().get<AnimatorComponent>(entity);

			if (renderComponent.hasAnimation)
			{

				std::string label = "CurrentAnim##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
				ImGui::DragInt(label.c_str(), &animComponent.currentAnim, 1, 0);

				label = "blendDuration##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
				ImGui::DragFloat(label.c_str(), &animComponent.blendDuration, 0.01, 0);
			}
			if (renderComponent.meshType == ECS::MESH_TYPE::LIGHT)
			{
				if (scene->GetRegistry().all_of<ECS::LightComponent>(entity))
				{
					ECS::LightComponent& lightComponent = scene->GetRegistry().get<ECS::LightComponent>(entity);
					std::string label;
					label = "Color##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
					ImGui::DragFloat3(label.c_str(), &lightComponent.color.x, 0.1, 0);
					label = "Radius##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
					ImGui::DragFloat(label.c_str(), &lightComponent.radius, 0.1, 0);
					label = "Strength##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
					ImGui::DragFloat(label.c_str(), &lightComponent.strength, 0.1, 0);
				}
			}

		}
		

		DirectX::XMMATRIX worldMatrix = m_closestTransform->worldMatrix;

		float matrix[16];
		DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)matrix, worldMatrix);

		if (ImGuizmo::Manipulate(
			(const float*)&viewMatrix,
			(const float*)&projMatrix,
			operation,
			ImGuizmo::LOCAL,
			matrix))
		{
			DirectX::XMMATRIX updatedMatix = DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4*)matrix);
			
			DirectX::XMVECTOR scaleVec, rotQuat, transVec;
			XMMatrixDecompose(&scaleVec, &rotQuat, &transVec, updatedMatix);

			DirectX::XMStoreFloat3(&m_closestTransform->position, transVec);
			DirectX::XMStoreFloat3(&m_closestTransform->scale, scaleVec);
			DirectX::XMStoreFloat4(&m_closestTransform->rotation, rotQuat);
		}
	}
	ImGui::End();
}

void GFXGui::UpdateAllEntities(ECS::SceneManager* sceneManager, UINT screenWidth, UINT screenHeight, Camera& camera)
{

	auto scene = sceneManager->GetCurrentScene();
	auto group = scene->GetRegistry().group<>(entt::get<ECS::TransformComponent, ECS::RenderComponent, AnimatorComponent>);

	ImGui::Begin(scene->GetName().c_str());

	for (auto [entity, transform, renderComponent, animComponent] : group.each())
	{
		std::string entityLabel = renderComponent.name + ": " + std::to_string(static_cast<uint32_t>(entity)) + "##" + std::to_string(static_cast<uint32_t>(entity));
		if (ImGui::CollapsingHeader(entityLabel.c_str()))
		{
			std::string posOffset = "Pos##" + std::to_string(static_cast<uint32_t>(entity));
			std::string scaleOffset = "Scale##" + std::to_string(static_cast<uint32_t>(entity));
			std::string rotOffset = "Rot##" + std::to_string(static_cast<uint32_t>(entity));

			ImGui::DragFloat3(posOffset.c_str(), &transform.position.x, 0.05f);
			ImGui::DragFloat3(rotOffset.c_str(), &transform.rotation.x, 0.05f);
			ImGui::DragFloat3(scaleOffset.c_str(), &transform.scale.x, 0.05f);

			if(renderComponent.hasAnimation)
			{
				std::string label = "CurrentAnim##" + std::to_string(static_cast<uint32_t>(entity));
				ImGui::DragInt(label.c_str(), &animComponent.currentAnim, 1, 0);

				label = "blendDuration##" + std::to_string(static_cast<uint32_t>(entity));
				ImGui::DragFloat(label.c_str(), &animComponent.blendDuration, 0.01, 0);
			}
		}
	}

	ImGui::End();
}

void GFXGui::BeginRender()
{
	ImGui_ImplSDL3_NewFrame();
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
