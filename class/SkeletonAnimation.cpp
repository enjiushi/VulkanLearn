#include "SkeletonAnimation.h"
#include "../Maths/AssimpDataConverter.h"
#include "scene.h"
#include "UniformData.h"
#include <codecvt>
#include <locale>

bool SkeletonAnimation::Init(const std::shared_ptr<SkeletonAnimation>& pSelf, const aiAnimation* pAssimpAnimation)
{
	if (!SelfRefBase<SkeletonAnimation>::Init(pSelf))
		return false;

	for (uint32_t i = 0; i < pAssimpAnimation->mNumChannels; i++)
	{
		std::wstring boneName = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(pAssimpAnimation->mChannels[i]->mNodeName.C_Str());
		uint32_t chunkIndex = UniformData::GetInstance()->GetPerFrameBoneUniforms()->AllocatePerObjectChunk();

		m_boneDiction[boneName] = chunkIndex;

		UniformData::GetInstance()->GetPerFrameBoneUniforms()->SetAnimationTransform(chunkIndex, AcquireTransformDQ(pAssimpAnimation->mChannels[i], 0));
	}

	return true;
}

std::shared_ptr<SkeletonAnimation> SkeletonAnimation::Create(const aiAnimation* pAssimpAnimation)
{
	std::shared_ptr<SkeletonAnimation> pSkeletonAnimation = std::make_shared<SkeletonAnimation>();
	if (pSkeletonAnimation != nullptr && pSkeletonAnimation->Init(pSkeletonAnimation, pAssimpAnimation))
		return pSkeletonAnimation;

	return nullptr;
}

std::vector<std::shared_ptr<SkeletonAnimation>> SkeletonAnimation::Create(const aiScene* pAssimpScene)
{
	std::vector<std::shared_ptr<SkeletonAnimation>> animations;
	for (uint32_t i = 0; i < pAssimpScene->mNumAnimations; i++)
		animations.push_back(Create(pAssimpScene->mAnimations[i]));

	return animations;
}

DualQuaternionf SkeletonAnimation::AcquireTransformDQ(const aiNodeAnim* pAssimpNodeAnim, uint32_t key)
{
	Quaternionf rotation = AssimpDataConverter::AcquireQuaternion(pAssimpNodeAnim->mRotationKeys[key].mValue);
	Vector3f scale = AssimpDataConverter::AcquireVector3(pAssimpNodeAnim->mScalingKeys[key].mValue);
	Vector3f position = AssimpDataConverter::AcquireVector3(pAssimpNodeAnim->mPositionKeys[key].mValue);
	return {};
}