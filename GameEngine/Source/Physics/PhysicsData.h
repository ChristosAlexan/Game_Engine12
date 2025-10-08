#pragma once
#include <PhysX/PxPhysicsAPI.h>

namespace PHYSICS
{
	enum PhysicsShapeEnum
	{
		PHYSICS_NONE = -1,
		PHYSICS_CUBE = 0,
		PHYSICS_PLANE = 1,
		PHYSICS_SPHERE = 2,
		PHYSICS_CAPSULE = 3,
		PHYSICS_CONVEXMESH = 4,
		PHYSICS_TRIANGLEMESH = 5
	};

	struct PhysicsTransform
	{

		physx::PxTransform transform;
		physx::PxVec3 position;
		physx::PxQuat rotation;
		physx::PxVec3 scale;
	};

	struct PhysicsComponent
	{
		PhysicsShapeEnum shapeType;
		physx::PxRigidDynamic* aActor = nullptr;
		physx::PxRigidStatic* aStaticActor = nullptr;
		physx::PxShape* aShape = nullptr;
		physx::PxReal mass = 0.0f;
		physx::PxReal density = 1.0f;
		physx::PxMaterial* aMaterial = nullptr;
		physx::PxReal radius = 1.0f;
		PhysicsTransform transform;
	};
}
