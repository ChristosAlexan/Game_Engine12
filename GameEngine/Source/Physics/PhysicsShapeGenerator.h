#pragma once
#include "PhysicsData.h"

namespace PHYSICS
{
	void PxCreateCube(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent);
	void PxCreatePlane(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent);
	void PxCreateSphere(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent);
	void PxCreateCapsule(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent);
	void PxCreateConvex(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent);
	void PxCreateTriangleMesh(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent);
}

