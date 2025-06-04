#include "SceneManager.h"
#include "MeshGenerators.h"
#include "ConstantBufferTypes.h"

namespace ECS
{
	SceneManager::SceneManager()
	{
	}
	void SceneManager::LoadAssets(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		CreateCubeEntity(device, cmdList);

	}
	ECS::EntityID SceneManager::CreateEntity()
	{
		EntityID id = m_NextEntityID++;
		return id;
	}
	void SceneManager::AddTransformComponent(EntityID id, const TransformComponent& transformComponent)
	{
		m_transformComponents.emplace(id, transformComponent);
	}
	void SceneManager::AddRenderComponent(EntityID id, const RenderComponent& renderComponent)
	{
		for (const auto& [existingID, _] : m_renderComponents)
		{
			if (existingID == id)
				return; // already added
		}

		m_renderComponents.emplace_back(std::make_pair(id, renderComponent));
	}

	void SceneManager::RenderEntities(ID3D12GraphicsCommandList* cmdList, DynamicUploadBuffer* dynamicCB, Camera& camera, float& dt)
	{
		for (const auto& [entityID, renderComponent] : m_renderComponents)
		{
			auto& tempTrans = m_transformComponents.at(entityID);
			tempTrans.position = DirectX::XMFLOAT3((entityID + 1) * 2 + cos(dt), 0, (entityID + 1) * 2);
			tempTrans.rotation = DirectX::XMFLOAT3((entityID + 1) * 2, (entityID + 1) * 2, (entityID + 1) * 2);
			UpdateTransform(entityID, tempTrans, dt);

			const auto& transform = m_transformComponents[entityID];

			// 1. Compute world matrix
			DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
				DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z) *
				DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);

			CB_VS_SimpleShader vsCB = {};
			vsCB.projectionMatrix = DirectX::XMMatrixTranspose(camera.GetProjectionMatrix());
			vsCB.viewMatrix = DirectX::XMMatrixTranspose(camera.GetViewMatrix());
			vsCB.worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);
			CB_PS_SimpleShader psCB = {};

			psCB.lightPos = DirectX::XMFLOAT4(3.0f, 5.0f, 1.0f, 1.0f);
			psCB.color = renderComponent.material->baseColor;
			cmdList->SetGraphicsRootConstantBufferView(0, dynamicCB->Allocate(vsCB));
			cmdList->SetGraphicsRootConstantBufferView(1, dynamicCB->Allocate(psCB));

			renderComponent.mesh->Draw(cmdList);
		}
	}

	void SceneManager::UpdateTransform(EntityID id, TransformComponent& trans, float dt)
	{
		trans.position = trans.position;
		trans.scale = trans.scale;
		trans.rotation = trans.rotation;
	}

	void SceneManager::Clear()
	{
	}
	
	ECS::EntityID SceneManager::CreateCubeEntity(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		EntityID id = CreateEntity();
		m_assetManager.GetOrLoadMesh("cube", device, cmdList);
	
		TransformComponent transformComponent;
		transformComponent.position = DirectX::XMFLOAT3(1, 0, 5);
		transformComponent.scale = DirectX::XMFLOAT3(1, 1, 1);
		transformComponent.rotation = DirectX::XMFLOAT3(0, 0, 0);
		AddTransformComponent(id, transformComponent);

		RenderComponent renderComponent;
		renderComponent.mesh = m_assetManager.m_meshes["cube"];
		renderComponent.material = std::make_shared <Material>();
		renderComponent.material->baseColor = DirectX::XMFLOAT4(1, 0, 0, 1);
		AddRenderComponent(id, renderComponent);

		return id;
	}
}
