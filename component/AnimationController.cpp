#include "AnimationController.h"
#include "../class/SkeletonAnimation.h"
#include "../class/SkeletonAnimationInstance.h"
#include "../Base/BaseObject.h"

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

}

void AnimationController::OnAddedToObjectInternal(const std::shared_ptr<BaseObject>& pObject)
{
	pObject->RegisterCallbackComponent(GetSelfSharedPtr());
}