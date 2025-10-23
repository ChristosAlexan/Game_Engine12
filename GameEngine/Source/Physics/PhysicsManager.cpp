#include "PhysicsManager.h"
#include "ErrorLogger.h"
#include <iostream>
#include "Scene.h"
#include "PhysicsData.h"
#include "PhysicsShapeGenerator.h"
#include "MathHelpers.h";

namespace PHYSICS
{
	PhysicsManager::PhysicsManager()
	{
	}

	PhysicsManager::~PhysicsManager()
	{
		ShutDown();
	}

	void PhysicsManager::Initialize()
	{
		static physx::PxDefaultErrorCallback defaultErrorCallback;
		static physx::PxDefaultAllocator defaultAllocatorCallback;

		m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, defaultAllocatorCallback, defaultErrorCallback);
		if (!m_foundation)
			ErrorLogger::Log("PxCreateFoundation failed!");

		bool recordMemoryAllocations = true;
		m_pvd = physx::PxCreatePvd(*m_foundation);

		const char* PVD_HOST = "Host";
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
		if (!transport)
			ErrorLogger::Log("PxDefaultPvdSocketTransportCreate failed!");

		physx::PxTolerancesScale scale;
		scale.length = 100;
		scale.speed = 981;
		m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, physx::PxTolerancesScale(), recordMemoryAllocations, m_pvd);

		if (!m_physics)
			ErrorLogger::Log("PxPhysics failed!");

		physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		m_dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = m_dispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		if (!sceneDesc.isValid())
			ErrorLogger::Log("PxSceneDesc failed!");

		m_aScene = m_physics->createScene(sceneDesc);

		m_manager = PxCreateControllerManager(*m_aScene);

		m_physicsDebugDraw = std::make_unique<PhysicsDebugDraw>(m_aScene);
	}

	void PhysicsManager::CreatePhysicsShapes(ECS::Scene* scene)
	{
		auto group = scene->GetRegistry().group<>(entt::get<ECS::TransformComponent, PhysicsComponent>);

		for (auto [entity, transformComponent, physicsComponent] : group.each())
		{
			switch (physicsComponent.shapeType)
			{
			case PHYSICS_CUBE:
				PxCreateCube(m_physics, m_aScene, physicsComponent);
				break;
			case PHYSICS_PLANE:
				PxCreatePlane(m_physics, m_aScene, physicsComponent);
				break;
			case PHYSICS_SPHERE:
				PxCreateSphere(m_physics, m_aScene, physicsComponent);
				break;
			case PHYSICS_CAPSULE:
				PxCreateCapsule(m_physics, m_aScene, physicsComponent);
				break;
			case PHYSICS_CONVEXMESH:
				PxCreateConvex(m_physics, m_aScene, physicsComponent);
				break;
			case PHYSICS_TRIANGLEMESH:
				PxCreateTriangleMesh(m_physics, m_aScene, physicsComponent);
				break;
			}
		}
	}

	void PhysicsManager::Update(ECS::Scene* scene, Camera& camera)
	{
		m_aScene->setVisualizationCullingBox(physx::PxBounds3(physx::PxVec3(camera.pos.x - 40.0f, camera.pos.y - 40.0f, camera.pos.z - 40.0f), physx::PxVec3(camera.pos.x + 40.0f, camera.pos.y + 40.0f, camera.pos.z + 40.0f)));
		m_aScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
		m_aScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 2.0f);

		auto group = scene->GetRegistry().group<>(entt::get<ECS::TransformComponent, PhysicsComponent>);
		for (auto [entity, transformComponent, physicsComponent] : group.each())
		{
			if(physicsComponent.mass > 0.0f)
			{
				if (m_bRunPhysics)
				{
					physicsComponent.transform.transform = physicsComponent.aActor->getGlobalPose();
					PhysXToTransform(physicsComponent.transform.transform, transformComponent);
				}
				else
				{
					physicsComponent.transform = TransformToPhysX(transformComponent);
					physicsComponent.aActor->setGlobalPose(physicsComponent.transform.transform);
				}
			}
			else
			{
				physicsComponent.transform = TransformToPhysX(transformComponent);
				physicsComponent.aStaticActor->setGlobalPose(physicsComponent.transform.transform);
			}
		}
	}

	bool PhysicsManager::Advance(float& dt, float& fps, Camera& camera)
	{
		if (!m_bRunPhysics)
			return false;

		m_stepSize = 1.0f / fps;
		m_accumulator += dt;
		if (m_accumulator < m_stepSize)
		{
			return false;
		}

		m_accumulator -= m_stepSize;

		m_aScene->simulate(m_stepSize);
		bool result = m_aScene->fetchResults(true);

		return true;
	}

	void PhysicsManager::ShutDown()
	{
		if (m_aScene)
			m_aScene->release();

		if (m_physics)
			m_physics->release();

		PxCloseExtensions();

		if (m_foundation)
			m_foundation->release();
	}

	physx::PxPhysics* PhysicsManager::GetPhysics() const
	{
		return m_physics;
	}

	physx::PxScene* PhysicsManager::GetScene() const
	{
		return m_aScene;
	}

	physx::PxFoundation* PhysicsManager::GetFoundation() const
	{
		return m_foundation;
	}

	PhysicsDebugDraw* PhysicsManager::GetDebugDraw() const
	{
		return m_physicsDebugDraw.get();
	}

}
