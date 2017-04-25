#pragma once
#include "DeviceMemoryManager.h"

class Buffer : DeviceObjectBase
{
public:
	~Buffer();

	bool Init(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag, const std::shared_ptr<DeviceMemoryManager>& pMemMgr, const void* pData = nullptr);

public:
	VkBuffer GetDeviceHandle() const { return m_buffer; }

protected:
	VkBuffer m_buffer;
};