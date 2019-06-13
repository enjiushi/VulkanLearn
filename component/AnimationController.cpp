#include "AnimationController.h"

DEFINITE_CLASS_RTTI(AnimationController, BaseComponent);

bool AnimationController::Init(const std::shared_ptr<AnimationController>& pAnimationController)
{
	if (!BaseComponent::Init(pAnimationController))
		return false;

	return true;
}