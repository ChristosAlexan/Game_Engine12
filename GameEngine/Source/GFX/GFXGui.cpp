#include "GFXGui.h"
#include "ErrorLogger.h"
#include "MathHelpers.h"
#include "RenderingManager.h"
#include "LightManager.h"
#include "PhysicsManager.h"

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
	init_info.RTVFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
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

void GFXGui::GeneralGuiSettings(ECS::SceneManager* sceneManager, Metrics& metrics)
{
	auto scene = sceneManager->GetCurrentScene();

	ImGui::Begin("GeneralSettings");
	ImGui::BeginDisabled();
	ImGui::Text("FPS: %.1f", metrics.avgFps);
	ImGui::Text("MS: %.1f", metrics.ms);
	ImGui::EndDisabled();
	ImGui::Checkbox("Run physics", &scene->GetPhysicsManager()->m_bRunPhysics);
	ImGui::Checkbox("Debug draw", &scene->GetRenderingManager()->m_bEnableDebugDraw);
	const uint32_t min_v = 0;
	const uint32_t max_v = 4;
	ImGui::SliderScalar("Vsync", ImGuiDataType_U32, &scene->GetRenderingManager()->GetDX12().m_vsync, &min_v, &max_v, "%u");
	ImGui::DragFloat3("AmbientColor", &scene->GetRenderingManager()->m_ambientColor.x, 0.01f);
	ImGui::DragFloat("Exposure", &scene->GetRenderingManager()->m_exposure, 0.01f);
	ImGui::DragFloat("Gamma", &scene->GetRenderingManager()->m_gamma, 0.01f);
	if (ImGui::TreeNode("AO Settings"))
	{
		ImGui::DragFloat("AO Radius", &scene->GetRenderingManager()->aoData.AORadius, 0.01f);
		ImGui::DragFloat("Normal Bias", &scene->GetRenderingManager()->aoData.NormalBias, 0.01f);
		ImGui::DragInt("Sample Count", &scene->GetRenderingManager()->aoData.SampleCount, 1, 1, 100);

		ImGui::TreePop();
	}
	ImGui::End();

	ImGui::Begin("Models");
	auto view = scene->GetRegistry().view<Model>();
	auto totalModels = view.size();
	std::vector<const char*> modelNames;
	static int currItem = -1;

	for (auto [entity, model] : view.each())
	{
		modelNames.push_back(model.name.c_str());
	}
	ImGui::ListBox(" ", &currItem, modelNames.data(), totalModels);

	ImGui::End();
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


		std::string entityLabel = m_closestEntityName + ": " + std::to_string(static_cast<uint32_t>(m_closestEntity));
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

		if (scene->GetRegistry().all_of<ECS::RenderComponent>(entity))
		{
			ECS::RenderComponent& renderComponent = scene->GetRegistry().get<ECS::RenderComponent>(entity);

			if (scene->GetRegistry().all_of<AnimatorComponent>(entity))
			{
				AnimatorComponent& animComponent = scene->GetRegistry().get<AnimatorComponent>(entity);

				if (renderComponent.hasAnimation)
				{
					std::string label = "CurrentAnim##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
					ImGui::DragInt(label.c_str(), &animComponent.currentAnim, 1, 0);

					label = "blendDuration##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
					ImGui::DragFloat(label.c_str(), &animComponent.blendDuration, 0.01, 0);
				}
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
					ImGui::DragFloat(label.c_str(), &lightComponent.radius, 0.01, 0);
					label = "Strength##" + std::to_string(static_cast<uint32_t>(m_closestEntity));		
					ImGui::DragFloat(label.c_str(), &lightComponent.strength, 0.01, 0);
					label = "Cutoff##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
					ImGui::DragFloat(label.c_str(), &lightComponent.cutoff, 0.01, 0);
					label = "ShadowSamples##" + std::to_string(static_cast<uint32_t>(m_closestEntity));
					ImGui::DragInt(label.c_str(), &lightComponent.ShadowSamples, 1, 0);
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

void GFXGui::SelectEntityList(ECS::SceneManager* sceneManager, UINT screenWidth, UINT screenHeight, Camera& camera)
{
	auto scene = sceneManager->GetCurrentScene();
	auto group = scene->GetRegistry().group<>(entt::get<ECS::TransformComponent, ECS::RenderComponent>);

	ImGui::Begin(scene->GetName().c_str());

	if (ImGui::BeginListBox("Entities"))
	{
		for (auto [entity, transform, renderComponent] : group.each())
		{
			std::string entityLabel = renderComponent.name + ": " + std::to_string((uint32_t)entity);
			bool isSelected = (m_closestEntity == entity);
			if (ImGui::Selectable(entityLabel.c_str(), isSelected))
			{
				m_closestEntity = entity;
				m_closestTransform = &transform;
				m_closestEntityName = renderComponent.name;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
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
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	style.WindowPadding = ImVec2(10.0f, 10.0f);
	style.FramePadding = ImVec2(8.0f, 5.0f);
	style.CellPadding = ImVec2(6.0f, 4.0f);
	style.ItemSpacing = ImVec2(8.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
	style.ScrollbarSize = 14.0f;
	style.GrabMinSize = 10.0f;

	style.WindowRounding = 8.0f;
	style.ChildRounding = 8.0f;
	style.FrameRounding = 5.0f;
	style.PopupRounding = 6.0f;
	style.ScrollbarRounding = 8.0f;
	style.GrabRounding = 5.0f;
	style.TabRounding = 5.0f;

	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.TabBorderSize = 0.0f;

	const ImVec4 bgDark = ImVec4(0.055f, 0.055f, 0.070f, 1.00f);
	const ImVec4 bgPanel = ImVec4(0.080f, 0.080f, 0.100f, 0.96f);
	const ImVec4 bgPanelLight = ImVec4(0.120f, 0.120f, 0.150f, 1.00f);
	const ImVec4 bgHover = ImVec4(0.200f, 0.200f, 0.250f, 1.00f);
	const ImVec4 bgActive = ImVec4(0.260f, 0.260f, 0.320f, 1.00f);

	const ImVec4 text = ImVec4(0.920f, 0.920f, 0.940f, 1.00f);
	const ImVec4 textMuted = ImVec4(0.560f, 0.560f, 0.600f, 1.00f);

	const ImVec4 accent = ImVec4(1.000f, 0.720f, 0.180f, 1.00f);
	const ImVec4 accentHover = ImVec4(1.000f, 0.820f, 0.320f, 1.00f);
	const ImVec4 accentActive = ImVec4(0.900f, 0.560f, 0.080f, 1.00f);

	const ImVec4 border = ImVec4(0.180f, 0.180f, 0.220f, 1.00f);
	const ImVec4 separator = ImVec4(0.250f, 0.250f, 0.300f, 1.00f);

	colors[ImGuiCol_Text] = text;
	colors[ImGuiCol_TextDisabled] = textMuted;

	colors[ImGuiCol_WindowBg] = bgDark;
	colors[ImGuiCol_ChildBg] = bgPanel;
	colors[ImGuiCol_PopupBg] = ImVec4(0.070f, 0.070f, 0.090f, 0.98f);
	colors[ImGuiCol_Border] = border;
	colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);

	colors[ImGuiCol_FrameBg] = bgPanelLight;
	colors[ImGuiCol_FrameBgHovered] = bgHover;
	colors[ImGuiCol_FrameBgActive] = bgActive;

	colors[ImGuiCol_TitleBg] = ImVec4(0.045f, 0.045f, 0.060f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.100f, 0.100f, 0.130f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.045f, 0.045f, 0.060f, 0.85f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.075f, 0.075f, 0.095f, 1.00f);

	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.045f, 0.045f, 0.060f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.230f, 0.230f, 0.280f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.330f, 0.330f, 0.390f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.430f, 0.430f, 0.500f, 1.00f);

	colors[ImGuiCol_CheckMark] = accent;
	colors[ImGuiCol_SliderGrab] = accent;
	colors[ImGuiCol_SliderGrabActive] = accentHover;

	colors[ImGuiCol_Button] = ImVec4(0.150f, 0.150f, 0.185f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.230f, 0.230f, 0.280f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.300f, 0.300f, 0.360f, 1.00f);

	colors[ImGuiCol_Header] = ImVec4(0.150f, 0.150f, 0.185f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.230f, 0.230f, 0.280f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.300f, 0.300f, 0.360f, 1.00f);
}
