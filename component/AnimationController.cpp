#include "AnimationController.h"
#include "../class/SkeletonAnimation.h"
#include "../class/SkeletonAnimationInstance.h"
#include "../Base/BaseObject.h"
#include "../Maths/DualQuaternion.h"

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

	return true;
}

void AnimationController::CallbackFunc(std::shared_ptr<BaseObject>& pObject)
{
	std::shared_ptr<SkeletonAnimation> pAnimation = m_pAnimationInstance->GetAnimation();
	auto iter = pAnimation->m_animationDataDiction[0].objectAnimationLookupTable.find(pObject->GetName());
	if (iter == pAnimation->m_animationDataDiction[0].objectAnimationLookupTable.end())
		return;

	Quaternionf& rotation = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[0].transform;
	Vector3f& translate = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].translationKeyFrames[0].transform;

	pObject->SetRotation(rotation);
	pObject->SetPos(translate);

	Matrix4f transform = pObject->GetWorldTransform();

	m_pAnimationInstance->SetBoneTransform(pObject->GetName(), DualQuaternionf(transform.RotationMatrix(), transform.TranslationVector()));
}

void AnimationController::OnAddedToObjectInternal(const std::shared_ptr<BaseObject>& pObject)
{
	pObject->RegisterCallbackComponent(GetSelfSharedPtr());
}