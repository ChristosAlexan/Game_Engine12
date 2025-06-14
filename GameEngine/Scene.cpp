#include "Scene.h"
#include "SceneManager.h"
#include "ErrorLogger.h"

namespace ECS
{
	Scene::Scene(const std::string& sceneName, std::shared_ptr<AssetManager> assetMgr, std::shared_ptr<MaterialManager> materialMgr,
		ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
		:m_sceneName(sceneName), m_assetManager(std::move(assetMgr)),
		m_materialManager(std::move(materialMgr)), m_device(device), m_cmdList(cmdList)
	{
		m_world = std::make_shared<ECSWorld>();
		m_entityFactory = std::make_unique<EntityFactory>(m_world.get(), m_device, m_cmdList);
	}

	ECS::EntityID Scene::CreateEntity()
	{
		EntityID id = m_NextEntityID++;
		return id;
	}

	void Scene::LoadMaterials()
	{
		MaterialDesc materialDesc = {};
		materialDesc.name = "DefaultMaterial";
		materialDesc.albedoTexturePath = "Data/Textures/Tex1/plasticpattern1-albedo.png";
		materialDesc.albedoTextureName = "defaultAlbedo";
		materialDesc.normalTexturePath = "Data/Textures/Tex1/plasticpattern1-normal2b.png";
		materialDesc.normalTextureName = "defaultNormal";
		materialDesc.roughnessTexturePath = "Data/Textures/Tex1/plasticpattern1-roughness2.png";
		materialDesc.roughnessTextureName = "defaultRougness";
		materialDesc.metalnessTexturePath = "Data/Textures/Tex1/plasticpattern1-metalness.png";
		materialDesc.metalnessTextureName = "defaultMetallic";

		m_materialManager->GetOrCreateMaterial(materialDesc, m_device, m_cmdList, g_descAllocator.get());
	}

	void Scene::LoadAssets()
	{
		for (int i = 0; i < 3; ++i)
		{
			EntityDesc entDesc = {};
			entDesc.meshType = CUBE;
			if (auto desc = m_materialManager->GetMaterialDescByName("DefaultMaterial"))
				entDesc.materialDesc = *desc;
			else
			{
				ErrorLogger::Log("Material descriptor not found");
				entDesc.materialDesc = MaterialDesc::Default();
			}

			m_entityFactory->CreateStaticMesh(this, entDesc);
		}

	}

	AssetManager* Scene::GetAssetManager() const
	{
		return m_assetManager.get();
	}

	MaterialManager* Scene::GetMaterialManager() const
	{
		return m_materialManager.get();
	}

	ECSWorld* Scene::GetWorld() const
	{
		return m_world.get();
	}

	void Scene::UpdateTransform(TransformComponent& trans)
	{
		trans.position = trans.position;
		trans.scale = trans.scale;
		trans.rotation = trans.rotation;
	}

	void Scene::Update(float dt)
	{
	}

	void Scene::Render(Camera& camera, DynamicUploadBuffer* dynamicCB)
	{
		for (const auto& [entityID, renderComponent] : GetWorld()->GetAllRenderComponents())
		{
			auto tempTrans = GetWorld()->GetComponent<TransformComponent>(entityID);
			UpdateTransform(*tempTrans);

			const auto& transform = GetWorld()->GetComponent<TransformComponent>(entityID);

			// 1. Compute world matrix
			DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixScaling(transform->scale.x, transform->scale.y, transform->scale.z) *
				DirectX::XMMatrixRotationRollPitchYaw(transform->rotation.x, transform->rotation.y, transform->rotation.z) *
				DirectX::XMMatrixTranslation(transform->position.x, transform->position.y, transform->position.z);

			CB_VS_SimpleShader vsCB = {};
			vsCB.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
			vsCB.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());
			vsCB.worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);
			CB_PS_SimpleShader psCB = {};

			psCB.lightPos = DirectX::XMFLOAT4(3.0f, 5.0f, 1.0f, 1.0f);
			psCB.color = renderComponent.material->baseColor;


			m_cmdList->SetGraphicsRootConstantBufferView(0, dynamicCB->Allocate(vsCB));
			m_cmdList->SetGraphicsRootConstantBufferView(1, dynamicCB->Allocate(psCB));

			m_materialManager->Bindtextures(renderComponent.material.get(), m_cmdList, 2);
			renderComponent.mesh->Draw(m_cmdList);
		}
	}
}

