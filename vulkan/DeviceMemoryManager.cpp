#include "DeviceMemoryManager.h"

DeviceMemoryManager::~DeviceMemoryManager()
{
	ReleaseMemory();
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

	vkFreeMemory(m_device, m_memory, nullptr);
	m_memory = 0;
}