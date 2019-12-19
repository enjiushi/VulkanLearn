#pragma once

#include "SkeletonAnimation.h"
#include "../Maths/DualQuaternion.h"

class Mesh;

class SkeletonAnimationInstance : public SelfRefBase<SkeletonAnimationInstance>
{
protected:
	bool Init(const std::shared_ptr<SkeletonAnimationInstance>& pSelf, const std::shared_ptr<SkeletonAnimation>& pSkeletonAnimation, const std::shared_ptr<Mesh>& pMesh);

public:
	static std::shared_ptr<SkeletonAnimationInstance> Create(const std::shared_ptr<SkeletonAnimation>& pSkeletonAnimation, const std::shared_ptr<Mesh>& pMesh);

public:
	std::shared_ptr<Mesh> GetMesh() const { return m_pMesh; }
	std::shared_ptr<SkeletonAnimation> GetAnimation() const { return m_pSkeletonAnimation; }
	void SetBoneTransform(std::size_t hashCode, uint32_t boneIndex, const DualQuaterniond& dq);
	uint32_t GetAnimationChunkIndex() const { return m_animationChunk; }

protected:
	std::shared_ptr<SkeletonAnimation>	m_pSkeletonAnimation;
	std::shared_ptr<Mesh>				m_pMesh;
	uint32_t							m_boneChunkIndexOffset;
	uint32_t							m_animationChunk;
};