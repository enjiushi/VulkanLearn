#pragma once

#include "../Maths/Matrix.h"
#include "../Base/Base.h"
#include "../Maths/DualQuaternion.h"
#include <map>

struct aiScene;
struct aiAnimation;
struct aiNodeAnim;

class SkeletonAnimation : public SelfRefBase<SkeletonAnimation>
{
protected:
	bool Init(const std::shared_ptr<SkeletonAnimation>& pSelf, const aiAnimation* pAssimpAnimation);

public:
	static std::shared_ptr<SkeletonAnimation> Create(const aiAnimation* pAssimpAnimation);
	static std::vector<std::shared_ptr<SkeletonAnimation>> Create(const aiScene* pAssimpScene);

protected:
	static DualQuaternionf AcquireTransformDQ(const aiNodeAnim* pAssimpNodeAnim, uint32_t key);

protected:
	std::map<std::wstring, uint32_t>	m_boneDiction;
};