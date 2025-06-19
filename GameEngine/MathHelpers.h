#pragma once
#include <DirectXMath.h>


struct Ray {
	DirectX::XMVECTOR origin;
	DirectX::XMVECTOR direction;
};


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

inline Ray RaycastPicking(UINT screenWidth, UINT screenHeight, Camera& camera)
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
