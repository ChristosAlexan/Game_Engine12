#include "EntityFactory.h"
#include "AssetManager.h"
#include "RenderingManager.h"
#include "Scene.h"
#include "MathHelpers.h"
#include "BLASBuilder.h"

namespace ECS
{
	EntityFactory::EntityFactory(entt::registry& registry, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
		:m_registry(&registry), m_device(device), m_cmdList(cmdList)
	{
	}

	entt::entity EntityFactory::AddEntity(Scene* scene, EntityDesc& entityDesc)
	{
		BLASBuilder blas_builder;
		std::unique_ptr<ECS::Material> mat;

		auto id = scene->CreateEntity();
	
		RenderComponent renderComponent = {};

		auto mesh = scene->GetAssetManager()->GetOrLoadMesh(scene, entityDesc, m_registry, id, m_device, m_cmdList);
		auto material = scene->GetMaterialManager()->GetOrCreateMaterial(entityDesc.materialDesc);

		
		renderComponent.mesh = mesh;
		renderComponent.material = material;
		renderComponent.name = entityDesc.name;
		renderComponent.hasAnimation = entityDesc.hasAnimation;
		renderComponent.hasTextures = entityDesc.hasTextures;
		renderComponent.meshType = entityDesc.meshType;

		// Generate AABB from mesh data offline
		GenerateAABB(entityDesc.transform.aabb, &renderComponent);

		if(renderComponent.meshType == ECS::MESH_TYPE::SKELETAL_MESH)
		{
			renderComponent.model = scene->GetAssetManager()->GetModel(entityDesc.name);

			renderComponent.blas = std::make_shared<BLAS>(blas_builder.Build(scene->GetRenderingManager()->GetDX12().GetDevice(), scene->GetRenderingManager()->GetDX12().GetCmdList(),
				renderComponent.mesh->vertexBuffer.GetVertexBufferVirtualAddress(), renderComponent.mesh->vertexCount, renderComponent.mesh->vertexBuffer.vbView.StrideInBytes,
				renderComponent.mesh->indexBuffer.GetIndexBufferVirtualAddress(), renderComponent.mesh->indexCount, renderComponent.mesh->indexBuffer.ibView.Format,
				D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE));

			renderComponent.skinningOutData.skinningVertexBufferFinalTransform.Initialize(scene->GetRenderingManager()->GetDX12().GetDevice(), 
				renderComponent.mesh->cpuMesh->vertices.size(), true);

			DescriptorAllocator::DescriptorHandle allocator = scene->GetRenderingManager()->GetDX12().GetDescriptorAllocator()->Allocate();
			renderComponent.skinningOutData.skinningCpuHandleFinalTransform = allocator.cpuHandle;
			renderComponent.skinningOutData.skinningGpuHandleFinalTransform = allocator.gpuHandle;

			renderComponent.skinningOutData.skinningVertexBufferFinalTransform.CreateUAV(scene->GetRenderingManager()->GetDX12().GetDevice(), 
				renderComponent.skinningOutData.skinningCpuHandleFinalTransform);
		}
		else if (renderComponent.meshType == ECS::MESH_TYPE::STATIC_MESH)
		{
			renderComponent.model = scene->GetAssetManager()->GetModel(entityDesc.name);

			renderComponent.blas = std::make_shared<BLAS>(blas_builder.Build(scene->GetRenderingManager()->GetDX12().GetDevice(), scene->GetRenderingManager()->GetDX12().GetCmdList(),
				renderComponent.mesh->vertexBuffer.GetVertexBufferVirtualAddress(), renderComponent.mesh->vertexCount, renderComponent.mesh->vertexBuffer.vbView.StrideInBytes,
				renderComponent.mesh->indexBuffer.GetIndexBufferVirtualAddress(), renderComponent.mesh->indexCount, renderComponent.mesh->indexBuffer.ibView.Format,
				D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE));
		}
		else if (entityDesc.meshType == ECS::MESH_TYPE::LIGHT)
		{
			LightComponent lightComponent;
			lightComponent.lightType = entityDesc.lightComponent.lightType;
			lightComponent.radius = entityDesc.lightComponent.radius;
			lightComponent.strength = entityDesc.lightComponent.strength;
			lightComponent.cutoff = entityDesc.lightComponent.cutoff;
			lightComponent.color = entityDesc.lightComponent.color;
			renderComponent.material->baseColor = DirectX::XMFLOAT4(lightComponent.color.x, lightComponent.color.y, lightComponent.color.z, 1.0f);

			m_registry->emplace<LightComponent>(id, lightComponent);
		}
		
		m_registry->emplace<RenderComponent>(id, renderComponent);
		m_registry->emplace<EntityDesc>(id, entityDesc);
		m_registry->emplace<TransformComponent>(id, entityDesc.transform);

		if(renderComponent.blas)
			scene->blas_total++;

		if (entityDesc.hasAnimation)
		{
			entityDesc.animComponent = AnimatorComponent{};
			m_registry->emplace<AnimatorComponent>(id, entityDesc.animComponent);
		}
			
		
		return id;
	}
}	

