#include "Fence.h"
#include "VKFenceGuardRes.h"

Fence::~Fence()
{
	vkDestroyFence(GetDevice()->GetDeviceHandle(), m_fence, nullptr);
}

bool Fence::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Fence>& pSelf)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	CHECK_VK_ERROR(vkCreateFence(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_fence));
	m_fenceState = FenceState::READ_FOR_USE;
	return true;
}

std::shared_ptr<Fence> Fence::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<Fence> pFence = std::make_shared<Fence>();
	if (pFence.get() && pFence->Init(pDevice, pFence))
		return pFence;
	return nullptr;
}

bool Fence::Wait()
{
	if (m_fenceState != FenceState::READ_FOR_SIGNAL)
		return false;

	CHECK_VK_ERROR(vkWaitForFences(GetDevice()->GetDeviceHandle(), 1, &m_fence, VK_TRUE, UINT64_MAX));
	m_fenceState = FenceState::SIGNALED;

	for (auto pRes : m_guardResources)
	{
		pRes->Release();
	}
	m_guardResources.clear();

	return true;
}

bool Fence::Reset()
{
	if (m_fenceState != FenceState::SIGNALED)
		return false;

	CHECK_VK_ERROR(vkResetFences(GetDevice()->GetDeviceHandle(), 1, &m_fence));
	m_fenceState = FenceState::READ_FOR_USE;

	return true;
}

bool Fence::AddResource(const std::shared_ptr<VKFenceGuardRes>& pResource)
{
	if (m_fenceState != READ_FOR_USE)
		return false;

	m_guardResources.push_back(pResource);
	pResource->GuardByFence(GetSelfSharedPtr());

	return true;
}

void Fence::MarkResouceOccupied()
{
	for (auto pRes : m_guardResources)
	{
		pRes->MarkOccupied();
	}
}