#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/DescriptorSetLayout.h"
#include "../vulkan/DescriptorSet.h"
#include "UniformBase.h"

bool UniformBase::Init(const std::shared_ptr<UniformBase>& pSelf)
{
	if (!SelfRefBase<UniformBase>::Init(pSelf))
		return false;

	return true;
}