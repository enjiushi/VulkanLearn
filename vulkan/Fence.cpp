#include "Fence.h"

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

	return true;
}

std::shared_ptr<Fence> Fence::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<Fence> pFence = std::make_shared<Fence>();
	if (pFence.get() && pFence->Init(pDevice, pFence))
		return pFence;
	return nullptr;
}