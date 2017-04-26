#include "Queue.h"

Queue::~Queue()
{
}

bool Queue::Init(const std::shared_ptr<Device>& pDevice, uint32_t queueIndex)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	vkGetDeviceQueue(pDevice->GetDeviceHandle(), pDevice->GetPhysicalDevice()->GetGraphicQueueIndex(), 0, &m_queue);
	return true;
}

std::shared_ptr<Queue> Queue::Create(const std::shared_ptr<Device>& pDevice, uint32_t queueIndex)
{
	std::shared_ptr<Queue> pQueue = std::make_shared<Queue>();
	if (pQueue.get() && pQueue->Init(pDevice, queueIndex))
		return pQueue;
	return nullptr;
}