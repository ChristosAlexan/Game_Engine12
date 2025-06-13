#pragma once
#include "MeshGenerators.h"
#include "EntityECS.h"
#include "ECSHeader.h"
#include "ECSWorld.h"

namespace ECS
{
	class Scene;

	class EntityFactory
	{
	public:
		EntityFactory(ECSWorld* world, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
		EntityID CreateStaticMesh(Scene* scene, EntityDesc& entityDesc);
		//EntityID CreateLightEntity(LightType type, const LightComponent& lightData);					   TODO
		//EntityID CreateCameraEntity(CameraType type);													   TODO
		//EntityID CreateSkeletalMesh(const std::string& modelName, const std::string& animName);		   TODO
		template<typename T>
		void AddComponent(EntityID id, const T& component)
		{
			m_world->AddComponent<T>(id, component);
		}

	private:
		ECSWorld* m_world = nullptr;
		ID3D12Device* m_device = nullptr;
		ID3D12GraphicsCommandList* m_cmdList = nullptr;

	};
}
