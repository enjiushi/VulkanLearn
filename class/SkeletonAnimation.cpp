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

	if (pAssimpScene->mNumAnimations == 0)
		return false;

	for (uint32_t i = 0; i < pAssimpScene->mNumAnimations; i++)
	{
		AnimationData animationData = {};
		AssemblyAnimationData(pAssimpScene->mAnimations[i], animationData);

		m_animationDataDiction.push_back(animationData);
		m_animationDataLookupTable[std::hash<std::wstring>()(animationData.animationName)] = m_animationDataDiction.size() - 1;
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

void SkeletonAnimation::AssemblyAnimationData(const aiAnimation* pAssimpAnimation, AnimationData& animationData)
{
	animationData.animationName = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(pAssimpAnimation->mName.C_Str());
	animationData.duration = pAssimpAnimation->mDuration / pAssimpAnimation->mTicksPerSecond;

	for (uint32_t i = 0; i < pAssimpAnimation->mNumChannels; i++)
	{
		ObjectAnimation objectAnimation = {};
		AssemblyObjectAnimation(pAssimpAnimation->mChannels[i], pAssimpAnimation->mTicksPerSecond, objectAnimation);

		animationData.objectAnimationDiction.push_back(objectAnimation);
		animationData.objectAnimationLookupTable[std::hash<std::wstring>()(objectAnimation.objectName)] = animationData.objectAnimationDiction.size() - 1;
	}
}

void SkeletonAnimation::AssemblyObjectAnimation(const aiNodeAnim* pAssimpNodeAnimation, float ticksPerSecond, ObjectAnimation& objectAnimation)
{
	objectAnimation.objectName = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(pAssimpNodeAnimation->mNodeName.C_Str());

	for (uint32_t i = 0; i < pAssimpNodeAnimation->mNumRotationKeys; i++)
	{
		RotationKeyFrame rotationKeyFrame = {};
		rotationKeyFrame.time = pAssimpNodeAnimation->mRotationKeys[i].mTime / ticksPerSecond;
		rotationKeyFrame.transform = AssimpDataConverter::AcquireQuaternion(pAssimpNodeAnimation->mRotationKeys[i].mValue).SinglePrecision();
		objectAnimation.rotationKeyFrames.push_back(rotationKeyFrame);
	}

	for (uint32_t i = 0; i < pAssimpNodeAnimation->mNumPositionKeys; i++)
	{
		TranslationKeyFrame translationKeyFrame = {};
		translationKeyFrame.time = pAssimpNodeAnimation->mRotationKeys[i].mTime;
		translationKeyFrame.transform = AssimpDataConverter::AcquireVector3(pAssimpNodeAnimation->mPositionKeys[i].mValue).SinglePrecision();
		objectAnimation.translationKeyFrames.push_back(translationKeyFrame);
	}

	for (uint32_t i = 0; i < pAssimpNodeAnimation->mNumScalingKeys; i++)
	{
		ScaleKeyFrame scaleKeyFrame = {};
		scaleKeyFrame.time = pAssimpNodeAnimation->mRotationKeys[i].mTime;
		scaleKeyFrame.transform = AssimpDataConverter::AcquireVector3(pAssimpNodeAnimation->mScalingKeys[i].mValue).SinglePrecision();
		objectAnimation.ScaleKeyFrames.push_back(scaleKeyFrame);
	}
}