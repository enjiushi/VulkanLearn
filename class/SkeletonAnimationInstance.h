#pragma once

#include "SkeletonAnimation.h"

class Mesh;

class SkeletonAnimationInstance : public SelfRefBase<SkeletonAnimationInstance>
{
protected:
	bool Init(const std::shared_ptr<SkeletonAnimationInstance>& pSelf, const std::shared_ptr<SkeletonAnimation>& pSkeletonAnimation, const std::shared_ptr<Mesh>& pMesh);

public:
	static std::shared_ptr<SkeletonAnimationInstance> Create(const std::shared_ptr<SkeletonAnimation>& pSkeletonAnimation, const std::shared_ptr<Mesh>& pMesh);

protected:
	std::shared_ptr<SkeletonAnimation>	m_pSkeletonAnimation;
	std::shared_ptr<Mesh>				m_pMesh;
	uint32_t							m_animationChunk;
};