#pragma once
#include "ComputeWrapper.h"

class ComputeSkinning : public ComputeWrapper
{
public:
	ComputeSkinning();
	void Initialize();
	virtual void Compute(ECS::Scene* scene) override;
};

