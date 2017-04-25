#pragma once
#include "DeviceObjectBase.h"

class DeviceMemoryManager : public DeviceObjectBase
{
public:
	DeviceMemoryManager() {}
	~DeviceMemoryManager();

public:
	bool AllocateMemory(uint32_t byteSize);

protected:
	void ReleaseMemory();

protected:
	VkDeviceMemory	m_memory;
};