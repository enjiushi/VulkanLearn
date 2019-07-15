#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"

class SkeletonAnimationInstance;

class AnimationController : public BaseComponent
{
	DECLARE_CLASS_RTTI(AnimationController, BaseComponent);

public:
	static std::shared_ptr<AnimationController> Create(const std::shared_ptr<SkeletonAnimationInstance>& pAnimationInstance = nullptr);
	~AnimationController() { delete[] m_pCurrentAnimationIndices; }

public:
	void CallbackFunc(std::shared_ptr<BaseObject>& pObject);
	std::shared_ptr<SkeletonAnimationInstance> GetAnimationInstance() const { return m_pAnimationInstance; }

	void Update() override;

protected:
	bool Init(const std::shared_ptr<AnimationController>& pAnimationController, const std::shared_ptr<SkeletonAnimationInstance>& pAnimationInstance = nullptr);

protected:
	void OnAddedToObjectInternal(const std::shared_ptr<BaseObject>& pObject) override;

protected:
	std::shared_ptr<SkeletonAnimationInstance>	m_pAnimationInstance;

	float		m_animationPlayedTime;

	uint32_t*	m_pCurrentAnimationIndices;
};