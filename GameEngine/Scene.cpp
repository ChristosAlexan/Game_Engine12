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
		m_registry = entt::registry{};
		m_entityFactory = std::make_unique<EntityFactory>(m_registry, m_device, m_cmdList);
	}

	entt::entity Scene::CreateEntity()
	{
		return m_registry.create();
	}

	void Scene::LoadMaterials()
	{
		MaterialDesc materialDesc = {};
		materialDesc.name = "DefaultMaterial";
		materialDesc.albedoTexturePath = "Data/Textures/Tex1/plasticpattern1-albedo.png";
		materialDesc.albedoTextureName = "defaultAlbedo";
		materialDesc.normalTexturePath = "Data/Textures/Tex1/plasticpattern1-normal2b.png";
		materialDesc.normalTextureName = "defaultNormal";
		materialDesc.metalRoughnessTexturePath = "Data/Textures/Tex1/plasticpattern1-roughness2.png";
		materialDesc.metalRoughnessTextureName = "defaultMetalRougness";
		materialDesc.useAlbedoMap = true;
		materialDesc.useNormalMap = true;
		materialDesc.useMetalRoughnessMap = true;
		materialDesc.tex_format = Texture12::TEXTURE_FORMAT::AUTO;

		m_materialManager->GetOrCreateMaterial(materialDesc);
	}

	void Scene::LoadAssets()
	{
		EntityDesc entDesc = {};

		for (int i = 0; i < 3; ++i)
		{
		
			if (auto desc = m_materialManager->GetMaterialDescByName("DefaultMaterial"))
				entDesc.materialDesc = *desc;
			else
			{
				ErrorLogger::Log("Material descriptor not found");
				entDesc.materialDesc = MaterialDesc::Default();
			}
			entDesc.meshType = CUBE;
			entDesc.name = "Cube";
			m_entityFactory->CreateMesh(this, entDesc);
		}

		//HELMET
		MaterialDesc materialDesc = {};
		materialDesc.name = "TestMaterial";
		materialDesc.albedoTexturePath = "G:/gltf models/glTF-Sample-Models-main/2.0/DamagedHelmet/glTF/Default_albedo.dds";
		materialDesc.albedoTextureName = "defaultAlbedo1";
		materialDesc.normalTexturePath = "G:/gltf models/glTF-Sample-Models-main/2.0/DamagedHelmet/glTF/Default_normal.dds";
		materialDesc.normalTextureName = "defaultNormal1";
		materialDesc.metalRoughnessTexturePath = "G:/gltf models/glTF-Sample-Models-main/2.0/DamagedHelmet/glTF/Default_metalRoughness.dds";
		materialDesc.metalRoughnessTextureName = "defaultMetalRougness1";
		materialDesc.useAlbedoMap = true;
		materialDesc.useNormalMap = true;
		materialDesc.useMetalRoughnessMap = true;
		materialDesc.tex_format = Texture12::TEXTURE_FORMAT::AUTO;

		entDesc.materialDesc = materialDesc;
		entDesc.meshType = STATIC_MESH;
		entDesc.name = "Helmet";
		entDesc.filePath = "G:/gltf models/glTF-Sample-Models-main/2.0/DamagedHelmet/glTF-Binary/DamagedHelmet.glb";
		entDesc.hasMaterial = true;
		m_entityFactory->CreateMesh(this, entDesc);

		//ROBO
		materialDesc.name = "RoboMaterial";
		materialDesc.albedoTexturePath = "G:/gltf models/glTF-Sample-Models-main/Custom/robo/robo_albedo.dds";
		materialDesc.albedoTextureName = "roboAlbedo";
		materialDesc.normalTexturePath = "G:/gltf models/glTF-Sample-Models-main/Custom/robo/robo_normal.dds";
		materialDesc.normalTextureName = "roboNormal";
		materialDesc.metalRoughnessTexturePath = "G:/gltf models/glTF-Sample-Models-main/Custom/robo/robo_metalness-robo_roughness.dds";
		materialDesc.metalRoughnessTextureName = "roboMetalRougness";
		materialDesc.useAlbedoMap = true;
		materialDesc.useNormalMap = true;
		materialDesc.useMetalRoughnessMap = true;
		materialDesc.tex_format = Texture12::TEXTURE_FORMAT::AUTO;

		entDesc.materialDesc = materialDesc;
		entDesc.meshType = STATIC_MESH;
		entDesc.name = "Robo";
		entDesc.filePath = "G:/gltf models/glTF-Sample-Models-main/Custom/robo/robo.glb";
		entDesc.hasMaterial = true;
		m_entityFactory->CreateMesh(this, entDesc);
	}

	AABB Scene::GenerateAABB(AABB& aabb, DirectX::XMMATRIX& worldMatrix, RenderComponent* renderComp)
	{
		DirectX::XMFLOAT3 minF, maxF;
		DirectX::XMVECTOR min = DirectX::XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
		DirectX::XMVECTOR max = DirectX::XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);

		for (const auto& vertex : renderComp->mesh->cpuMesh.vertices)
		{
			DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&vertex.pos);
			min = DirectX::XMVectorMin(min, pos);
			max = DirectX::XMVectorMax(max, pos);
		}

		aabb.min = min;
		aabb.max = max;
		DirectX::XMStoreFloat3(&minF, aabb.min);
		XMStoreFloat3(&maxF, aabb.max);

		// 8 corners of the AABB
		DirectX::XMVECTOR corners[8] = {
			DirectX::XMVectorSet(minF.x, minF.y, minF.z, 1.0f),
			DirectX::XMVectorSet(maxF.x, minF.y, minF.z, 1.0f),
			DirectX::XMVectorSet(minF.x, maxF.y, minF.z, 1.0f),
			DirectX::XMVectorSet(maxF.x, maxF.y, minF.z, 1.0f),
			DirectX::XMVectorSet(minF.x, minF.y, maxF.z, 1.0f),
			DirectX::XMVectorSet(maxF.x, minF.y, maxF.z, 1.0f),
			DirectX::XMVectorSet(minF.x, maxF.y, maxF.z, 1.0f),
			DirectX::XMVectorSet(maxF.x, maxF.y, maxF.z, 1.0f),
		};

		DirectX::XMVECTOR newMin = DirectX::XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
		DirectX::XMVECTOR newMax = DirectX::XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);

		// Transform all corners and find new min/max
		for (int i = 0; i < 8; ++i) {
			DirectX::XMVECTOR cornerWorld = XMVector3TransformCoord(corners[i], worldMatrix);
			newMin = DirectX::XMVectorMin(newMin, cornerWorld);
			newMax = DirectX::XMVectorMax(newMax, cornerWorld);
		}

		return { newMin, newMax };
	}

	AABB Scene::GetWorldAABB(TransformComponent* trans, RenderComponent* renderComp)
	{
		return GenerateAABB(trans->aabb, trans->worldMatrix, renderComp);
	}

	const std::string Scene::GetName() const
	{
		return m_sceneName;
	}

	AssetManager* Scene::GetAssetManager() const
	{
		return m_assetManager.get();
	}

	MaterialManager* Scene::GetMaterialManager() const
	{
		return m_materialManager.get();
	}

	entt::registry& Scene::GetRegistry()
	{
		return m_registry;
	}

	void Scene::Update(float dt)
	{
	}

	void Scene::Render(Camera& camera, DynamicUploadBuffer* dynamicCB)
	{
		auto renderGroup = GetRegistry().group<TransformComponent, RenderComponent>();

		for (auto [entity, transform, renderComponent] : renderGroup.each())
		{
			DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&transform.position);
			DirectX::XMVECTOR rot_vec = DirectX::XMLoadFloat4(&transform.rotation);
			DirectX::XMVECTOR scale_vec = DirectX::XMLoadFloat3(&transform.scale);

			DirectX::XMMATRIX scale_mat = DirectX::XMMatrixScalingFromVector(scale_vec);
			DirectX::XMMATRIX rot_mat = DirectX::XMMatrixRotationQuaternion(rot_vec);
			DirectX::XMMATRIX pos_mat = DirectX::XMMatrixTranslationFromVector(pos_vec);

			transform.worldMatrix = scale_mat * rot_mat * pos_mat;

			CB_VS_SimpleShader vsCB = {};
			vsCB.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
			vsCB.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());
			vsCB.worldMatrix = DirectX::XMMatrixTranspose(transform.worldMatrix);

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

