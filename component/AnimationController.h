#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"

class AnimationController : public BaseComponent
{
	DECLARE_CLASS_RTTI(AnimationController, BaseComponent);

public:
	static std::shared_ptr<AnimationController> Create();

protected:
	bool Init(const std::shared_ptr<AnimationController>& pAnimationController);
};