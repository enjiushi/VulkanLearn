#include "SkeletonAnimation.h"
#include "../Maths/AssimpDataConverter.h"
#include "scene.h"
#include "UniformData.h"
#include <codecvt>
#include <locale>

bool SkeletonAnimation::Init(const std::shared_ptr<SkeletonAnimation>& pSelf, const aiScene* pAssimpScene)
{
	if (!SelfRefBase<SkeletonAnimation>::Init(pSelf))
		return false;

	for (uint32_t i = 0; i < pAssimpScene->mNumAnimations; i++)
	{
		std::wstring animationName = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(pAssimpScene->mAnimations[i]->mName.C_Str());
		m_animationDataDiction[animationName] = {};
		AssemblyAnimationData(pAssimpScene->mAnimations[i], m_animationDataDiction[animationName]);
	}

	return true;
}

std::shared_ptr<SkeletonAnimation> SkeletonAnimation::Create(const aiScene* pAssimpScene)
{
	std::shared_ptr<SkeletonAnimation> pSkeletonAnimation = std::make_shared<SkeletonAnimation>();
	if (pSkeletonAnimation != nullptr && pSkeletonAnimation->Init(pSkeletonAnimation, pAssimpScene))
		return pSkeletonAnimation;

	return nullptr;
}

DualQuaternionf SkeletonAnimation::AcquireTransformDQ(const aiNodeAnim* pAssimpNodeAnim, uint32_t key)
{
	Quaternionf rotation = AssimpDataConverter::AcquireQuaternion(pAssimpNodeAnim->mRotationKeys[key].mValue);
	Vector3f position = AssimpDataConverter::AcquireVector3(pAssimpNodeAnim->mPositionKeys[key].mValue);

	return {};
}

void SkeletonAnimation::AssemblyAnimationData(const aiAnimation* pAssimpAnimation, AnimationData& animationData)
{
	animationData.animationName = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(pAssimpAnimation->mName.C_Str());
	animationData.duration = pAssimpAnimation->mDuration / pAssimpAnimation->mTicksPerSecond;

	for (uint32_t i = 0; i < pAssimpAnimation->mNumChannels; i++)
	{
		std::wstring objectName = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(pAssimpAnimation->mChannels[i]->mNodeName.C_Str());
		animationData.objectAnimationDiction[objectName] = {};
		AssemblyObjectAnimation(pAssimpAnimation->mChannels[i], animationData.objectAnimationDiction[objectName]);
	}
}

void SkeletonAnimation::AssemblyObjectAnimation(const aiNodeAnim* pAssimpNodeAnimation, ObjectAnimation& objectAnimation)
{
	objectAnimation.objectName = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(pAssimpNodeAnimation->mNodeName.C_Str());

	for (uint32_t i = 0; i < pAssimpNodeAnimation->mNumRotationKeys; i++)
	{
		RotationKeyFrame rotationKeyFrame = {};
		rotationKeyFrame.time = pAssimpNodeAnimation->mRotationKeys[i].mTime;
		rotationKeyFrame.transform = AssimpDataConverter::AcquireQuaternion(pAssimpNodeAnimation->mRotationKeys[i].mValue);
		objectAnimation.rotationKeyFrames.push_back(rotationKeyFrame);
	}

	for (uint32_t i = 0; i < pAssimpNodeAnimation->mNumPositionKeys; i++)
	{
		TranslationKeyFrame translationKeyFrame = {};
		translationKeyFrame.time = pAssimpNodeAnimation->mRotationKeys[i].mTime;
		translationKeyFrame.transform = AssimpDataConverter::AcquireVector3(pAssimpNodeAnimation->mPositionKeys[i].mValue);
		objectAnimation.translationKeyFrames.push_back(translationKeyFrame);
	}

	for (uint32_t i = 0; i < pAssimpNodeAnimation->mNumScalingKeys; i++)
	{
		ScaleKeyFrame scaleKeyFrame = {};
		scaleKeyFrame.time = pAssimpNodeAnimation->mRotationKeys[i].mTime;
		scaleKeyFrame.transform = AssimpDataConverter::AcquireVector3(pAssimpNodeAnimation->mScalingKeys[i].mValue);
		objectAnimation.ScaleKeyFrames.push_back(scaleKeyFrame);
	}
}