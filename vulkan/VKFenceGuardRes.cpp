#include "VKFenceGuardRes.h"
#include "Fence.h"

bool VKFenceGuardRes::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<VKFenceGuardRes>& pSelf)
{
	if (!DeviceObjectBase<VKFenceGuardRes>::Init(pDevice, pSelf))
		return false;

	return true;
}

void VKFenceGuardRes::WaitForRelease()
{
	if (m_pGuardFence.expired())
		return;

	m_pGuardFence.lock()->Wait();
}