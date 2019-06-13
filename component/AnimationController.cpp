#include "AnimationController.h"

bool AnimationController::Init(const std::shared_ptr<AnimationController>& pAnimationController)
{
	if (!AnimationController::Init(pAnimationController))
		return false;

	return true;
}