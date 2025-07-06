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
		std::string fpath = ".//Save files/" + GetName();
		m_saveLoadSystem.LoadScene(this, fpath);
	}


	void Scene::Update(float dt)
	{
		GetAnimationManager()->Update(dt, this);
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

	AnimationManager* Scene::GetAnimationManager() const
	{
		return m_animationManager.get();
	}

	EntityFactory* Scene::GetEntityFactory() const
	{
		return m_entityFactory.get();
	}

	void Scene::Render(Camera& camera, DynamicUploadBuffer* dynamicCB)
	{
		auto animView = GetRegistry().view<Model>();
		auto renderGroup = GetRegistry().group<TransformComponent, RenderComponent, AnimatorComponent>();

		for (auto [entity, transform, renderComponent, animComponent] : renderGroup.each())
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

			CB_VS_AnimationShader skinningCB = {};
			if (renderComponent.hasAnimation && !animComponent.finalTransforms.empty())
			{
				memcpy(skinningCB.skinningMatrix, animComponent.finalTransforms.data(), sizeof(skinningCB.skinningMatrix));

			}
			skinningCB.HasAnim = renderComponent.hasAnimation;

			CB_PS_SimpleShader psCB = {};
			psCB.lightPos = DirectX::XMFLOAT4(3.0f, 5.0f, 1.0f, 1.0f);
			psCB.color = renderComponent.material->baseColor;


			
			m_cmdList->SetGraphicsRootConstantBufferView(0, dynamicCB->Allocate(vsCB));
			m_cmdList->SetGraphicsRootConstantBufferView(1, dynamicCB->Allocate(psCB));
			m_cmdList->SetGraphicsRootConstantBufferView(3, dynamicCB->Allocate(skinningCB));

			m_materialManager->Bindtextures(renderComponent.material.get(), m_cmdList, 2);
			renderComponent.mesh->Draw(m_cmdList);
		}
	}
}

