#pragma once
#include <d3d12.h>

namespace ECS
{
	class Scene;
}
class ComputeWrapper
{
public:
	ComputeWrapper();
	void Initialize(UINT threadsPerGroup[3]);
	virtual void Compute(ECS::Scene* scene);
protected:
	UINT m_threadsPerGroup[3];
};

