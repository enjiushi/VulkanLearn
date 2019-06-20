#pragma once

#include "SkeletonAnimation.h"

class SkeletonAnimationInstance : public SelfRefBase<SkeletonAnimationInstance>
{
protected:
	bool Init(const std::shared_ptr<SkeletonAnimationInstance>& pSelf, const std::shared_ptr<SkeletonAnimation>& pSkeletonAnimation);

public:
	static std::shared_ptr<SkeletonAnimationInstance> Create(const std::shared_ptr<SkeletonAnimation>& pSkeletonAnimation);

protected:
	std::shared_ptr<SkeletonAnimation>		m_pSkeletonAnimation;

	// After each bone was interpolated both between key frames and animations
	// Final per frame bone transformation will be updated in PerFrameBoneUniforms
	// And their chunk indices will be stored in a diction, which is indexed by bone index
	std::vector<uint32_t>	m_perFrameBoneChunkIndexDiction;

	// As each animation instance could map to different animation even different animation from different mesh
	// Their bone count could be different
	// An offset is enough to find where it locates
	uint32_t				m_dictionOffset;
};