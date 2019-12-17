#pragma once

#include "../Maths/Matrix.h"
#include "../Base/Base.h"
#include "../Maths/DualQuaternion.h"
#include <unordered_map>

struct aiScene;
struct aiAnimation;
struct aiNodeAnim;

class SkeletonAnimationInstance;
class AnimationController;

typedef struct _RotationKeyFrame
{
	double			time;			// When does this key frame start
	Quaterniond		transform;		// Rotation
}RotationKeyFrame;

typedef struct _TranslationKeyFrame
{
	double			time;			// When does this key frame start
	Vector3d		transform;		// Translation
}TranslationKeyFrame;

typedef struct _ScaleKeyFrame
{
	double			time;			// When does this key frame start
	Vector3d		transform;		// Scale
}ScaleKeyFrame;

typedef struct _ObjectAnimation
{
	std::wstring					objectName;
	std::vector<RotationKeyFrame>	rotationKeyFrames;		// A list of rotation key frames for this object
	std::vector<TranslationKeyFrame>translationKeyFrames;	// A list of translation key frames for this object
	std::vector<ScaleKeyFrame>		ScaleKeyFrames;			// A list of scale key frames for this object
}ObjectAnimation;

typedef struct _AnimationData
{
	std::wstring					animationName;			// Animation name
	float							duration;				// Total duration of this animation
	std::vector<ObjectAnimation>	objectAnimationDiction;
	std::unordered_map<std::size_t, uint32_t> objectAnimationLookupTable;	// Using this to lookup specific index in object animation dictionary
}AnimationData;

class SkeletonAnimation : public SelfRefBase<SkeletonAnimation>
{
protected:
	bool Init(const std::shared_ptr<SkeletonAnimation>& pSelf, const aiScene* pAssimpScene);

public:
	static std::shared_ptr<SkeletonAnimation> Create(const aiScene* pAssimpScene);

protected:
	static void AssemblyAnimationData(const aiAnimation* pAssimpAnimation, AnimationData& animationData);
	static void AssemblyObjectAnimation(const aiNodeAnim* pAssimpNodeAnimation, double ticksPerSecond, ObjectAnimation& objectAnimation);

protected:
	std::vector<AnimationData>						m_animationDataDiction;			// Entire animation dictionary, containing all the data of current assimp scene's animation
	std::unordered_map<std::size_t, uint32_t>		m_animationDataLookupTable;		// Using this to lookup specific index in animation dictionary

	friend class SkeletonAnimationInstance;
	friend class AnimationController;
};