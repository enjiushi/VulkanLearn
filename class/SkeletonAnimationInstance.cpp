#include "SkeletonAnimationInstance.h"
#include "UniformData.h"
#include "../common/Macros.h"

bool SkeletonAnimationInstance::Init(const std::shared_ptr<SkeletonAnimationInstance>& pSelf, const std::shared_ptr<SkeletonAnimation>& pSkeletonAnimation)
{
	if (!SelfRefBase<SkeletonAnimationInstance>::Init(pSelf))
		return false;

	//m_perFrameBoneChunkIndexDiction.resize(pSkeletonAnimation->m_animationDataDiction[0].objectAnimationDiction.size());

	//for each (auto animationData in pSkeletonAnimation->m_animationDataDiction)
	//{
	//	ASSERTION(m_perFrameBoneChunkIndexDiction.size() == animationData.objectAnimationDiction.size());

	//	for each (auto objectAnimation in animationData.objectAnimationDiction)
	//	{
	//		uint32_t chunkIndex = UniformData::GetInstance()->GetPerFrameBoneUniforms()->AllocatePerObjectChunk();

	//		m_perFrameBoneChunkIndexDiction[UniformData::getbonei()->get]
	//		perAnimationBoneData.chunkIndexLookupTable.resize()
	//		
	//		//UniformData::GetInstance()->GetPerFrameBoneUniforms()->SetAnimationTransform(m_boneAnimationTransformDiction[objectAnimation.first], {});
	//	}

	//	m_chunkIndexIndirectBuffer.push_back(perAnimationBoneData);

	return true;
}

std::shared_ptr<SkeletonAnimationInstance> SkeletonAnimationInstance::Create(const std::shared_ptr<SkeletonAnimation>& pSkeletonAnimation)
{
	std::shared_ptr<SkeletonAnimationInstance> pSkeletonAnimationInstance = std::make_shared<SkeletonAnimationInstance>();
	if (pSkeletonAnimationInstance != nullptr && pSkeletonAnimationInstance->Init(pSkeletonAnimationInstance, pSkeletonAnimation))
		return pSkeletonAnimationInstance;

	return nullptr;
}