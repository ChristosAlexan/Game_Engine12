#include "ComputeWrapper.h"
#include "Scene.h"

ComputeWrapper::ComputeWrapper()
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		m_threadsPerGroup[i] = 0;
	}
}

void ComputeWrapper::Initialize(UINT threadsPerGroup[3])
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		m_threadsPerGroup[i] = threadsPerGroup[i];
	}
}

void ComputeWrapper::Compute(ECS::Scene* scene)
{
}
