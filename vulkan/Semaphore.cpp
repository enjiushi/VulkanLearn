#include "Semaphore.h"

Semaphore::~Semaphore()
{
	vkDestroySemaphore(GetDevice()->GetDeviceHandle(), m_semaphore, nullptr);
}

bool Semaphore::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Semaphore>& pSelf)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	CHECK_VK_ERROR(vkCreateSemaphore(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_semaphore));

	return true;
}

std::shared_ptr<Semaphore> Semaphore::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<Semaphore> pSemaphore = std::make_shared<Semaphore>();
	if (pSemaphore.get() && pSemaphore->Init(pDevice, pSemaphore))
		return pSemaphore;
	return nullptr;
}