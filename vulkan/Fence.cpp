#include "Fence.h"

#define UINT64_MAX       0xffffffffffffffffui64

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
	m_signaled = true;
	return true;
}

std::shared_ptr<Fence> Fence::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<Fence> pFence = std::make_shared<Fence>();
	if (pFence.get() && pFence->Init(pDevice, pFence))
		return pFence;
	return nullptr;
}

void Fence::Wait()
{
	if (m_signaled)
		return;

	CHECK_VK_ERROR(vkWaitForFences(GetDevice()->GetDeviceHandle(), 1, &m_fence, VK_TRUE, UINT64_MAX));
	m_signaled = true;
}

void Fence::Reset()
{
	if (!m_signaled)
		return;

	CHECK_VK_ERROR(vkResetFences(GetDevice()->GetDeviceHandle(), 1, &m_fence));
	m_signaled = false;
}