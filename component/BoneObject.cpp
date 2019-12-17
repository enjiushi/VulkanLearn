#include "BoneObject.h"
#include "AnimationController.h"
#include "../Base/BaseObject.h"
#include "../class/UniformData.h"
#include "../Maths/DualQuaternion.h"

DEFINITE_CLASS_RTTI(BoneObject, BaseComponent);

bool BoneObject::Init(const std::shared_ptr<BoneObject>& pSelf, std::weak_ptr<AnimationController> pRootBone, uint32_t boneIndex, const DualQuaterniond& boneOffset)
{
	if (!BaseComponent::Init(pSelf))
		return false;

	m_pRootBone = pRootBone;
	m_boneIndex = boneIndex;
	m_boneOffset = boneOffset;

	return true;
}

std::shared_ptr<BoneObject> BoneObject::Create(std::weak_ptr<AnimationController> pRootBone, uint32_t boneIndex, const DualQuaterniond& boneOffset)
{
	std::shared_ptr<BoneObject> pBoneObject = std::make_shared<BoneObject>();
	if (pBoneObject.get() && pBoneObject->Init(pBoneObject, pRootBone, boneIndex, boneOffset))
		return pBoneObject;
	return nullptr;
}

void BoneObject::OnAnimationUpdate()
{
	if (m_pRootBone.expired())
		return;

	m_pRootBone.lock()->UpdateBoneTransform(GetBaseObject());
}

void BoneObject::OnPreRender()
{
	m_pRootBone.lock()->SyncBoneTransformToUniform(GetBaseObject(), m_boneIndex, m_boneOffset);
}