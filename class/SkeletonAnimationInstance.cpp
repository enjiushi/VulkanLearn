#include "SkeletonAnimationInstance.h"
#include "UniformData.h"
#include "../Maths/AssimpDataConverter.h"
#include "../common/Macros.h"
#include "Mesh.h"

bool SkeletonAnimationInstance::Init(const std::shared_ptr<SkeletonAnimationInstance>& pSelf, const std::shared_ptr<SkeletonAnimation>& pSkeletonAnimation, const std::shared_ptr<Mesh>& pMesh)
{
	if (!SelfRefBase<SkeletonAnimationInstance>::Init(pSelf))
		return false;

	// No animation of input mesh? quit
	if (!pMesh->ContainBoneData())
		return false;

	m_pSkeletonAnimation = pSkeletonAnimation;
	m_pMesh = pMesh;

	uint32_t boneCount;
	ASSERTION(UniformData::GetInstance()->GetPerMeshUniforms()->GetBoneCount(pMesh->GetMeshChunkIndex(), boneCount));
	m_animationChunk = UniformData::GetInstance()->GetPerAnimationUniforms()->AllocateConsecutiveChunks(boneCount);

	uint32_t boneIndex;
	for each (auto objectAnimation in pSkeletonAnimation->m_animationDataDiction[0].objectAnimationDiction)
	{
		// Animation bone and mesh bone doesn't match? quit
		if (!UniformData::GetInstance()->GetPerMeshUniforms()->GetBoneIndex(pMesh->GetMeshChunkIndex(), objectAnimation.objectName, boneIndex))
			return false;

		DualQuaternionf dq = { objectAnimation.rotationKeyFrames[0].transform, objectAnimation.translationKeyFrames[0].transform };

		UniformData::GetInstance()->GetPerAnimationUniforms()->SetBoneTransform(m_animationChunk, objectAnimation.objectName, boneIndex, dq);
	}

	return true;
}

std::shared_ptr<SkeletonAnimationInstance> SkeletonAnimationInstance::Create(const std::shared_ptr<SkeletonAnimation>& pSkeletonAnimation, const std::shared_ptr<Mesh>& pMesh)
{
	std::shared_ptr<SkeletonAnimationInstance> pSkeletonAnimationInstance = std::make_shared<SkeletonAnimationInstance>();
	if (pSkeletonAnimationInstance != nullptr && pSkeletonAnimationInstance->Init(pSkeletonAnimationInstance, pSkeletonAnimation, pMesh))
		return pSkeletonAnimationInstance;

	return nullptr;
}