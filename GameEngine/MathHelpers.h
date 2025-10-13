#pragma once
#include <DirectXMath.h>
#include "TransformECS.h"
#include "Camera.h"
#include "imgui_internal.h"
#include "RenderingECS.h"
#include <iostream>
#include "Physics/PhysicsData.h"

struct Ray 
{
	DirectX::XMVECTOR origin;
	DirectX::XMVECTOR direction;
};

struct Frustum 
{
	DirectX::XMVECTOR planes[6];
};

inline Frustum ExtractFrustum(const DirectX::XMMATRIX& viewProj)
{
	Frustum frustum;

	// Transpose for easier access if needed
	DirectX::XMFLOAT4X4 m;
	DirectX::XMStoreFloat4x4(&m, viewProj);

	// Left plane: m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0], m[3][3] + m[3][0]
	frustum.planes[0] = DirectX::XMPlaneNormalize(
		DirectX::XMVectorSet(m._14 + m._11, m._24 + m._21, m._34 + m._31, m._44 + m._41));

	// Right plane
	frustum.planes[1] = DirectX::XMPlaneNormalize(
		DirectX::XMVectorSet(m._14 - m._11, m._24 - m._21, m._34 - m._31, m._44 - m._41));

	// Bottom plane
	frustum.planes[2] = DirectX::XMPlaneNormalize(
		DirectX::XMVectorSet(m._14 + m._12, m._24 + m._22, m._34 + m._32, m._44 + m._42));

	// Top plane
	frustum.planes[3] = DirectX::XMPlaneNormalize(
		DirectX::XMVectorSet(m._14 - m._12, m._24 - m._22, m._34 - m._32, m._44 - m._42));

	// Near plane
	frustum.planes[4] = DirectX::XMPlaneNormalize(
		DirectX::XMVectorSet(m._13, m._23, m._33, m._43));

	// Far plane
	frustum.planes[5] = DirectX::XMPlaneNormalize(
		DirectX::XMVectorSet(m._14 - m._13, m._24 - m._23, m._34 - m._33, m._44 - m._43));

	return frustum;
}

inline bool IsAABBInFrustum(const ECS::AABB& aabb, const Frustum& frustum)
{
	for (int i = 0; i < 6; ++i)
	{
		// Get the positive vertex (furthest point in plane normal direction)
		DirectX::XMVECTOR positiveVertex = DirectX::XMVectorSelect(
			aabb.min,
			aabb.max,
			DirectX::XMVectorGreater(frustum.planes[i], DirectX::XMVectorZero())
		);

		// If the positive vertex is outside (negative side), AABB is outside
		float distance = DirectX::XMVectorGetX(
			DirectX::XMPlaneDotCoord(frustum.planes[i], positiveVertex));

		if (distance < 0.0f)
			return false; // AABB is completely outside this plane
	}

	return true;
}

inline DirectX::XMFLOAT3 QuaternionToEulerAngles(DirectX::XMVECTOR q)
{
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(q);
	float pitch, yaw, roll;

	pitch = asinf(-R.r[2].m128_f32[1]); // -m13

	if (cosf(pitch) > 1e-6f)
	{
		yaw = atan2f(R.r[2].m128_f32[0], R.r[2].m128_f32[2]); // m31 / m33
		roll = atan2f(R.r[0].m128_f32[1], R.r[1].m128_f32[1]); // m12 / m22
	}
	else
	{
		yaw = atan2f(-R.r[1].m128_f32[0], R.r[0].m128_f32[0]); // -m21 / m11
		roll = 0.0f;
	}

	return DirectX::XMFLOAT3(pitch, yaw, roll); // radians
}



inline bool IntersectsAABB(const Ray& ray, const ECS::AABB& box, float& outDistance)
{
	using namespace DirectX;

	float tMin = 0.0f;
	float tMax = FLT_MAX;

	for (int i = 0; i < 3; ++i)
	{
		float rayOrigin = XMVectorGetByIndex(ray.origin, i);
		float rayDir = XMVectorGetByIndex(ray.direction, i);
		float boxMin = XMVectorGetByIndex(box.min, i);
		float boxMax = XMVectorGetByIndex(box.max, i);

		if (fabs(rayDir) < 1e-8f)
		{
			if (rayOrigin < boxMin || rayOrigin > boxMax)
				return false;
		}
		else
		{
			float ood = 1.0f / rayDir;
			float t1 = (boxMin - rayOrigin) * ood;
			float t2 = (boxMax - rayOrigin) * ood;

			if (t1 > t2) std::swap(t1, t2);

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
				return false;
		}
	}

	// Only accept forward-facing hits
	if (tMin < 0.0f)
		return false;

	outDistance = tMin;
	return true;
}

inline inline Ray RaycastPicking(UINT screenWidth, UINT screenHeight, Camera& camera)
{
	ImVec2 mousePos = ImGui::GetMousePos();
	float ndcX = (2.0f * mousePos.x) / screenWidth - 1.0f;
	float ndcY = 1.0f - (2.0f * mousePos.y) / screenHeight;

	DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(nullptr, camera.GetViewMatrix());
	DirectX::XMMATRIX invProj = DirectX::XMMatrixInverse(nullptr, camera.GetProjectionMatrix());

	DirectX::XMVECTOR rayClip = DirectX::XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);
	DirectX::XMVECTOR rayEye = XMVector3TransformCoord(rayClip, invProj);

	DirectX::XMVECTOR rayDir = DirectX::XMVector3Normalize(XMVector3TransformNormal(rayEye, invView));
	DirectX::XMVECTOR rayOrigin = camera.GetPositionVector();

	return { rayOrigin, rayDir };
}

inline void GenerateAABB(ECS::AABB& aabb, ECS::RenderComponent* renderComp)
{
	DirectX::XMVECTOR min = DirectX::XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
	DirectX::XMVECTOR max = DirectX::XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);

	for (const auto& vertex : renderComp->mesh->cpuMesh->vertices)
	{
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&vertex.pos);
		min = DirectX::XMVectorMin(min, pos);
		max = DirectX::XMVectorMax(max, pos);
	}

	aabb.min = min;
	aabb.max = max;
}

inline ECS::AABB UpdateAABB(ECS::AABB& aabb, DirectX::XMMATRIX& worldMatrix, ECS::RenderComponent* renderComp)
{
	DirectX::XMFLOAT3 minF, maxF;

	DirectX::XMStoreFloat3(&minF, aabb.min);
	DirectX::XMStoreFloat3(&maxF, aabb.max);

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

inline ECS::AABB GetWorldAABB(ECS::TransformComponent* trans, ECS::RenderComponent* renderComp)
{
	return UpdateAABB(trans->aabb, trans->worldMatrix, renderComp);
}

inline PHYSICS::PhysicsTransform TransformToPhysX(const ECS::TransformComponent& transform)
{
	PHYSICS::PhysicsTransform pxTrans;

	pxTrans.position = physx::PxVec3(transform.position.x, transform.position.y, transform.position.z);
	pxTrans.scale = physx::PxVec3(transform.scale.x, transform.scale.y, transform.scale.z);
	pxTrans.rotation = physx::PxQuat(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);

	pxTrans.transform = physx::PxTransform(pxTrans.position, pxTrans.rotation);

	return pxTrans;
}

inline DirectX::XMFLOAT4X4 MatrixToFloat4x4(const DirectX::XMMATRIX inMatrix)
{
	DirectX::XMFLOAT4X4 out;

	DirectX::XMStoreFloat4x4(&out, inMatrix);

	return out;
}

inline ECS::TransformComponent PhysXToTransform(const physx::PxTransform pxTransform, ECS::TransformComponent& transform)
{
	transform.position = DirectX::XMFLOAT3(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z);
	//pxTrans.scale = physx::PxVec3(transform.scale.x, transform.scale.y, transform.scale.z);
	 
	transform.rotation = DirectX::XMFLOAT4(pxTransform.q.x, pxTransform.q.y, pxTransform.q.z, pxTransform.q.w);

	return transform;
}

inline void PrintMatrix(const DirectX::XMMATRIX& mat, const char* label = "")
{
	DirectX::XMFLOAT4X4 m;
	DirectX::XMStoreFloat4x4(&m, mat);

	std::ostringstream oss{};
	if (label && *label)
		oss << label << ":\n";

	for (int i = 0; i < 4; ++i)
	{
		oss << "[ " << m.m[i][0] << ", " << m.m[i][1] << ", "
			<< m.m[i][2] << ", " << m.m[i][3] << " ]\n";
	}

	OutputDebugStringA(oss.str().c_str());
}

inline void DebugMatrix(const DirectX::XMMATRIX& mat, const char* name = "Matrix")
{
	DirectX::XMFLOAT4X4 m;
	DirectX::XMStoreFloat4x4(&m, mat);

	std::cout << name << ":\n";
	std::cout << std::fixed << std::setprecision(4); // nice formatting

	for (int row = 0; row < 4; ++row)
	{
		std::cout << "[ ";
		for (int col = 0; col < 4; ++col)
		{
			std::cout << std::setw(8) << m.m[row][col] << " ";
		}
		std::cout << "]\n";
	}
	std::cout << std::endl;
}