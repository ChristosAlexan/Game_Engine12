#include "Scene.h"
#include "SceneManager.h"
#include "ErrorLogger.h"



namespace ECS
{
	Scene::Scene(const std::string& sceneName, std::shared_ptr<AssetManager> assetMgr, std::shared_ptr<MaterialManager> materialMgr, 
		std::shared_ptr<RenderingManager> renderingManager,
		ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
		:m_sceneName(sceneName), m_assetManager(assetMgr),
		m_materialManager(materialMgr), m_renderingManager(renderingManager)
	{
		m_registry = entt::registry{};
		m_entityFactory = std::make_unique<EntityFactory>(m_registry, device, cmdList);
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

	void Scene::Update(float dt, Camera& camera, DynamicUploadBuffer* dynamicCB, ID3D12GraphicsCommandList* cmdList)
	{
		auto group = GetRegistry().group<TransformComponent, RenderComponent, AnimatorComponent>();
		for (auto [entity, transformComponent, renderComponent, animatorComponent] : group.each())
		{
			GetAnimationManager()->Update(dt, this, transformComponent, renderComponent, animatorComponent);
			GetRenderingManager()->Render(this, camera, dynamicCB, cmdList, transformComponent, renderComponent, animatorComponent);
		}
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

	RenderingManager* Scene::GetRenderingManager() const
	{
		return m_renderingManager.get();
	}

	EntityFactory* Scene::GetEntityFactory() const
	{
		return m_entityFactory.get();
	}
	SaveLoadSystem& Scene::GetSaveLoadSystems()
	{
		return m_saveLoadSystem;
	}
}

