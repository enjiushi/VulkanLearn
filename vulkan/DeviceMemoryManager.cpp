#include "DeviceMemoryManager.h"

DeviceMemoryManager::~DeviceMemoryManager()
{
	ReleaseMemory();
}

bool DeviceMemoryManager::AllocateMemory(uint32_t byteSize)
{
	ReleaseMemory();
}

void DeviceMemoryManager::ReleaseMemory()
{
	if (m_memory == 0)
		return;

	vkFreeMemory(m_device, m_memory, nullptr);
	m_memory = 0;
}