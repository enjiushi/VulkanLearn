#include "DeviceMemoryManager.h"

DeviceMemoryManager::~DeviceMemoryManager()
{
	ReleaseMemory();
}

bool DeviceMemoryManager::Init(const std::shared_ptr<Device>& pDevice)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	return true;
}

std::shared_ptr<DeviceMemoryManager> DeviceMemoryManager::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<DeviceMemoryManager> pMgr = std::make_shared<DeviceMemoryManager>(DeviceMemoryManager());
	if (pMgr.get() && pMgr->Init(pDevice))
		return pMgr;
	return nullptr;
}

bool DeviceMemoryManager::AllocateMemory(uint32_t byteSize)
{
	ReleaseMemory();

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = byteSize;

	return true;
}

void DeviceMemoryManager::ReleaseMemory()
{
	if (m_memory == 0)
		return;

	vkFreeMemory(GetDevice()->GetDeviceHandle(), m_memory, nullptr);
	m_memory = 0;
}