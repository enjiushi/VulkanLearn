#include "VKGPUSyncRes.h"

bool VKGPUSyncRes::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<VKGPUSyncRes>& pSelf)
{
	if (!VKFenceGuardRes::Init(pDevice, pSelf))
		return false;

	return true;
}