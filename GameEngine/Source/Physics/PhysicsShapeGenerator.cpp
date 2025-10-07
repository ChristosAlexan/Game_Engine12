#include "PhysicsShapeGenerator.h"
#include <algorithm>
#include <iostream>

namespace PHYSICS
{
	void PxCreateCube(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent)
	{
		physicsComponent.transform.scale.x = std::clamp(physicsComponent.transform.scale.x, 0.01f, 10000.0f);
		physicsComponent.transform.scale.y = std::clamp(physicsComponent.transform.scale.y, 0.01f, 10000.0f);
		physicsComponent.transform.scale.z = std::clamp(physicsComponent.transform.scale.z, 0.01f, 10000.0f);
		physicsComponent.aMaterial = physics->createMaterial(1.0f, 1.0f, 0.1f);

		if (physicsComponent.mass > 0.0f)
		{
			physicsComponent.aActor = physics->createRigidDynamic(physx::PxTransform(physicsComponent.transform.position));
			physicsComponent.aActor->setMass(physicsComponent.mass);

			physicsComponent.aShape = physx::PxRigidActorExt::createExclusiveShape(*physicsComponent.aActor, physx::PxBoxGeometry(physicsComponent.transform.scale), *physicsComponent.aMaterial);

			physx::PxRigidBodyExt::updateMassAndInertia(*physicsComponent.aActor, physicsComponent.density);

			physicsComponent.aActor->getShapes(&physicsComponent.aShape, physicsComponent.aActor->getNbShapes());

			physicsComponent.aShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
			physicsComponent.transform.transform = physicsComponent.aActor->getGlobalPose();

			scene->addActor(*physicsComponent.aActor);
		}
		else
		{
			physicsComponent.aStaticActor = physx::PxCreateStatic(*physics, physx::PxTransform(physicsComponent.transform.position), 
				physx::PxBoxGeometry(physicsComponent.transform.scale), *physicsComponent.aMaterial);

			physicsComponent.aStaticActor->getShapes(&physicsComponent.aShape, physicsComponent.aStaticActor->getNbShapes());

			physicsComponent.aShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
			physicsComponent.transform.transform = physicsComponent.aStaticActor->getGlobalPose();

			scene->addActor(*physicsComponent.aStaticActor);
		}
	}

	void PxCreatePlane(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent)
	{
		physicsComponent.aMaterial = physics->createMaterial(1.0f, 1.0f, 0.9f);
		physicsComponent.aStaticActor = physx::PxCreatePlane(*physics, physx::PxPlane(physicsComponent.transform.position, physx::PxVec3(0, 1, 0)), *physicsComponent.aMaterial);

		physicsComponent.aStaticActor->getShapes(&physicsComponent.aShape, physicsComponent.aStaticActor->getNbShapes());

		physicsComponent.aShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
		physicsComponent.transform.transform = physicsComponent.aStaticActor->getGlobalPose();

		scene->addActor(*physicsComponent.aStaticActor);
	}

	void PxCreateSphere(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent)
	{
		physicsComponent.aMaterial = physics->createMaterial(1.0f, 1.0f, 0.1f);

		if (physicsComponent.mass > 0.0f)
		{
			physicsComponent.aActor = physics->createRigidDynamic(physx::PxTransform(physicsComponent.transform.position));
			physx::PxTransform relativePose(physx::PxQuat(physx::PxPi, physx::PxVec3(0, 0, 1)));
			physicsComponent.aActor->setMass(physicsComponent.mass);

			physicsComponent.aShape = physx::PxRigidActorExt::createExclusiveShape(*physicsComponent.aActor, physx::PxSphereGeometry(physicsComponent.radius), *physicsComponent.aMaterial);
			physicsComponent.aShape->setLocalPose(relativePose);

			physx::PxRigidBodyExt::updateMassAndInertia(*physicsComponent.aActor, physicsComponent.density);

			physicsComponent.aActor->getShapes(&physicsComponent.aShape, physicsComponent.aActor->getNbShapes());
			physicsComponent.aShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
			
			physicsComponent.transform.transform = physicsComponent.aActor->getGlobalPose();

			scene->addActor(*physicsComponent.aActor);
		}
		else
		{
			physicsComponent.aStaticActor = physx::PxCreateStatic(*physics, physx::PxTransform(physicsComponent.transform.position, physicsComponent.transform.rotation), physx::PxSphereGeometry(physicsComponent.radius), *physicsComponent.aMaterial);

			physicsComponent.aStaticActor->getShapes(&physicsComponent.aShape, physicsComponent.aStaticActor->getNbShapes());
			physicsComponent.aShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);

			physicsComponent.transform.transform = physicsComponent.aStaticActor->getGlobalPose();
			
			scene->addActor(*physicsComponent.aStaticActor);
		}
	}

	void PxCreateCapsule(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent)
	{
		if (physicsComponent.mass > 0.0f)
		{
			physicsComponent.aActor = physics->createRigidDynamic(physx::PxTransform(physicsComponent.transform.position, physicsComponent.transform.rotation));
			physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
			physicsComponent.aActor->setMass(physicsComponent.mass);
			physicsComponent.aMaterial = physics->createMaterial(1.0f, 1.0f, 0.1f);

			physicsComponent.aShape = physx::PxRigidActorExt::createExclusiveShape(*physicsComponent.aActor, physx::PxCapsuleGeometry(physicsComponent.radius, physicsComponent.radius), *physicsComponent.aMaterial);
			physicsComponent.aShape->setLocalPose(relativePose);

			physx::PxRigidBodyExt::updateMassAndInertia(*physicsComponent.aActor, physicsComponent.density);

			physicsComponent.aActor->getShapes(&physicsComponent.aShape, physicsComponent.aActor->getNbShapes());
			physicsComponent.aShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);

			physicsComponent.transform.transform = physicsComponent.aActor->getGlobalPose();
	
			scene->addActor(*physicsComponent.aActor);
		}
		else
		{
			physicsComponent.aMaterial = physics->createMaterial(1.0f, 1.0f, 0.9f);
			physicsComponent.aStaticActor = physx::PxCreateStatic(*physics, physx::PxTransform(physicsComponent.transform.position, physicsComponent.transform.rotation), physx::PxCapsuleGeometry(physicsComponent.radius, physicsComponent.radius), *physicsComponent.aMaterial);

			physicsComponent.aShape = physx::PxRigidActorExt::createExclusiveShape(*physicsComponent.aStaticActor, physx::PxCapsuleGeometry(physicsComponent.radius, physicsComponent.radius), *physicsComponent.aMaterial);
			
			physicsComponent.aStaticActor->getShapes(&physicsComponent.aShape, physicsComponent.aStaticActor->getNbShapes());

			physicsComponent.aShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
			physicsComponent.transform.transform = physicsComponent.aStaticActor->getGlobalPose();

			scene->addActor(*physicsComponent.aStaticActor);
		}
	}

	void PxCreateConvex(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent)
	{
		
	}

	void PxCreateTriangleMesh(physx::PxPhysics* physics, physx::PxScene* scene, PhysicsComponent& physicsComponent)
	{

	}
}