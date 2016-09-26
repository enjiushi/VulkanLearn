#pragma once
#include "VulkanBase.h"

class DeviceMemoryManager : public VulkanBase
{
public:
	DeviceMemoryManager() : VulkanBase(), m_memory(0) {}
	DeviceMemoryManager(VkDevice device) : VulkanBase(device), m_memory(0) {}
	~DeviceMemoryManager();

public:
	bool AllocateMemory(uint32_t byteSize);

protected:
	void ReleaseMemory();

protected:
	VkDeviceMemory	m_memory;
};