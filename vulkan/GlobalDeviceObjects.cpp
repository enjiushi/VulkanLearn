#include "GlobalDeviceObjects.h"
#include "Queue.h"
#include "CommandPool.h"
#include "DeviceMemoryManager.h"
#include "StagingBufferManager.h"

bool GlobalDeviceObjects::Init(const std::shared_ptr<Device>& pDevice)
{
	m_pGraphicQueue = Queue::Create(pDevice, pDevice->GetPhysicalDevice()->GetGraphicQueueIndex());
	m_pPresentQueue = Queue::Create(pDevice, pDevice->GetPhysicalDevice()->GetPresentQueueIndex());

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = pDevice->GetPhysicalDevice()->GetGraphicQueueIndex();
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	m_pMainThreadCmdPool = CommandPool::Create(pDevice, commandPoolCreateInfo);

	m_pDeviceMemMgr = DeviceMemoryManager::Create(pDevice);

	m_pStaingBufferMgr = StagingBufferManager::Create(pDevice);

	return true;
}