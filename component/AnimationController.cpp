#include "AnimationController.h"
#include "../class/SkeletonAnimation.h"
#include "../class/SkeletonAnimationInstance.h"
#include "../Base/BaseObject.h"
#include "../Maths/DualQuaternion.h"
#include "../class/UniformData.h"
#include "../class/Mesh.h"
#include "../class/Timer.h"

DEFINITE_CLASS_RTTI(AnimationController, BaseComponent);

std::shared_ptr<AnimationController> AnimationController::Create(const std::shared_ptr<SkeletonAnimationInstance>& pAnimationInstance)
{
	std::shared_ptr<AnimationController> pAnimationController = std::make_shared<AnimationController>();
	if (pAnimationController.get() && pAnimationController->Init(pAnimationController, pAnimationInstance))
		return pAnimationController;

	return nullptr;
}

bool AnimationController::Init(const std::shared_ptr<AnimationController>& pAnimationController, const std::shared_ptr<SkeletonAnimationInstance>& pAnimationInstance)
{
	if (!BaseComponent::Init(pAnimationController))
		return false;

	m_pAnimationInstance = pAnimationInstance;

	m_pCurrentAnimationIndices = new uint32_t[m_pAnimationInstance->GetAnimation()->m_animationDataDiction[0].objectAnimationDiction.size()];
	memset(m_pCurrentAnimationIndices, 0, sizeof(uint32_t) * m_pAnimationInstance->GetAnimation()->m_animationDataDiction[0].objectAnimationDiction.size());

	return true;
}

void AnimationController::Update()
{
	float elapsed = Timer::GetElapsedTime();
	m_animationPlayedTime += elapsed / 1000.0f;
	m_animationPlayedTime = fmod(m_animationPlayedTime, m_pAnimationInstance->GetAnimation()->m_animationDataDiction[0].duration);
}

void AnimationController::CallbackFunc(std::shared_ptr<BaseObject>& pObject)
{
	DualQuaternionf boneOffsetDQ;

	// Check if current object is a bone
	if (!UniformData::GetInstance()->GetPerBoneIndirectUniforms()->GetBoneTransform(m_pAnimationInstance->GetMesh()->GetMeshBoneChunkIndexOffset(), pObject->GetNameHashCode(), boneOffsetDQ))
		return;

	std::shared_ptr<SkeletonAnimation> pAnimation = m_pAnimationInstance->GetAnimation();
	auto iter = pAnimation->m_animationDataDiction[0].objectAnimationLookupTable.find(pObject->GetName());

	// If current object contains animation information, it's local transform will be changed accordingly
	if (iter != pAnimation->m_animationDataDiction[0].objectAnimationLookupTable.end())
	{
		uint32_t keyFrameCount = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames.size();
		uint32_t currentKeyFrameIndex = m_pCurrentAnimationIndices[iter->second];

		// Check if there is actually a next key frame
		if (m_pCurrentAnimationIndices[iter->second] + 1 < keyFrameCount
			// And if current keyframe is done, move to next possible animation
			&& m_animationPlayedTime > pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[m_pCurrentAnimationIndices[iter->second] + 1].time)
		{
			do
			{
				currentKeyFrameIndex += 1;
			} while (currentKeyFrameIndex < keyFrameCount
				&& m_animationPlayedTime > pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[currentKeyFrameIndex].time);

			currentKeyFrameIndex--;
			m_pCurrentAnimationIndices[iter->second] = currentKeyFrameIndex;
		}
		// If the whole animation is done, then start over again(FIXME: should add animation state and configuration later to configure this)
		else if (m_animationPlayedTime < pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[m_pCurrentAnimationIndices[iter->second]].time)
		{
			currentKeyFrameIndex = 0;

			do
			{
				currentKeyFrameIndex += 1;
			} while (m_animationPlayedTime > pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[currentKeyFrameIndex].time);

			currentKeyFrameIndex--;
			m_pCurrentAnimationIndices[iter->second] = currentKeyFrameIndex;
		}

		uint32_t nextKeyFrameIndex = currentKeyFrameIndex + 1;

		// To deal with the situation that some object's animation is shorter than the others, therefore it'll keep last key frame until current animation is done
		if (nextKeyFrameIndex >= keyFrameCount)
			nextKeyFrameIndex = currentKeyFrameIndex;

		float currentAnimationTime = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[currentKeyFrameIndex].time;
		float nextAnimationTime = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[nextKeyFrameIndex].time;
		float factor = (m_animationPlayedTime - currentAnimationTime) / (nextAnimationTime - currentAnimationTime);

		// To deal with the situation that some object's animation is shorter than the others, therefore it'll keep last key frame until current animation is done
		factor = factor > 1.0f ? 1.0f : factor;

		Quaternionf& currentRotation = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[currentKeyFrameIndex].transform;
		Vector3f& currentTranslation = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].translationKeyFrames[currentKeyFrameIndex].transform;

		Quaternionf& nextRotation = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[nextKeyFrameIndex].transform;
		Vector3f& nextTranslation = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].translationKeyFrames[nextKeyFrameIndex].transform;

		Quaternionf blendRotation = Quaternionf::SLerp(currentRotation, nextRotation, factor);
		Vector3f blendTranslation = currentTranslation * (1.0f - factor) + nextTranslation * factor;

		pObject->SetRotation(blendRotation);
		pObject->SetPos(blendTranslation);
	}

	Matrix4f animationRootTransform = GetBaseObject()->GetWorldTransform();

	Matrix4f boneTransform = pObject->GetWorldTransform();
	boneTransform = animationRootTransform.Inverse() * boneTransform;

	boneTransform = boneTransform * Matrix4f(boneOffsetDQ.AcquireRotation().Matrix(), boneOffsetDQ.AcquireTranslation());

	m_pAnimationInstance->SetBoneTransform(pObject->GetNameHashCode(), DualQuaternionf(boneTransform.RotationMatrix(), boneTransform.TranslationVector()));
}

void AnimationController::OnAddedToObjectInternal(const std::shared_ptr<BaseObject>& pObject)
{
	pObject->RegisterCallbackComponent(GetSelfSharedPtr());
}