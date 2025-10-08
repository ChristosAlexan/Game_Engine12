#pragma once
#include <PxPhysicsAPI.h>
#include"PxDefaultErrorCallback.h"
#include"PxDefaultAllocator.h"
#include "../Camera.h"
#include "PhysicsDebugDraw.h"

namespace ECS
{
	class Scene;
}

namespace PHYSICS
{
	class PhysicsManager
	{
	public:
		PhysicsManager();
		~PhysicsManager();

		void Initialize();
		void CreatePhysicsShapes(ECS::Scene* scene);
		void Update(ECS::Scene* scene);
		bool Advance(float& dt, float& fps, Camera& camera);
		void ShutDown();

		physx::PxPhysics* GetPhysics() const;
		physx::PxScene* GetScene() const;
		physx::PxFoundation* GetFoundation() const;
		PhysicsDebugDraw* GetDebugDraw() const;
		
	private:
		physx::PxDefaultAllocator      m_allocator;
		physx::PxDefaultErrorCallback  m_errorCallback;
		physx::PxFoundation* m_foundation = nullptr;
		physx::PxPhysics* m_physics = nullptr;
		physx::PxScene* m_aScene = nullptr;
		physx::PxPvd* m_pvd = nullptr;
		physx::PxDefaultCpuDispatcher* m_dispatcher = nullptr;
		physx::PxControllerManager* m_manager = nullptr;
		std::unique_ptr<PhysicsDebugDraw> m_physicsDebugDraw;

		float m_accumulator = 0.0f;
		float m_stepSize = 1.0f / 60.0f;

	public:
		bool m_bRunPhysics = false;
	};

}

