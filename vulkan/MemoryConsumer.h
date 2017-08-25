#pragma once
#include <stdint.h>
#include "vulkan.h"

class MemoryConsumer
{
public:
	virtual uint32_t GetMemoryProperty() const = 0;
	virtual VkMemoryRequirements GetMemoryReqirments() const = 0;
	virtual void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes) = 0;

protected:
	virtual void BindMemory(VkDeviceMemory memory, uint32_t offset) const = 0;
	friend class DeviceMemoryManager;
};