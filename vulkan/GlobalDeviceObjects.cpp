#include "GlobalDeviceObjects.h"
#include "Queue.h"
#include "CommandPool.h"
#include "DeviceMemoryManager.h"
#include "StagingBufferManager.h"
#include "SwapChain.h"

bool GlobalDeviceObjects::Init(const std::shared_ptr<Device>& pDevice)
{
	m_pGraphicQueue = Queue::Create(pDevice, pDevice->GetPhysicalDevice()->GetGraphicQueueIndex());
	m_pPresentQueue = Queue::Create(pDevice, pDevice->GetPhysicalDevice()->GetPresentQueueIndex());

	m_pMainThreadCmdPool = CommandPool::Create(pDevice);

	m_pDeviceMemMgr = DeviceMemoryManager::Create(pDevice);

	m_pStaingBufferMgr = StagingBufferManager::Create(pDevice);

	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	info.size = UNIFORM_BUFFER_SIZE;
	m_pBigUniformBuffer = Buffer::Create(pDevice, info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	m_pSwapChain = SwapChain::Create(pDevice);

	return true;
}

GlobalDeviceObjects* GloablObjects()
{
	return GlobalDeviceObjects::GetInstance();
}