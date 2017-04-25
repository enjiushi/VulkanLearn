#pragma once
#include "DeviceMemoryManager.h"

class Buffer : DeviceObjectBase
{
public:
	~Buffer();

	bool Init(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag, const std::shared_ptr<DeviceMemoryManager>& pMemMgr, const void* pData = nullptr);

public:
	VkBuffer GetDeviceHandle() const { return m_buffer; }
	const VkBufferCreateInfo& GetBufferInfo() const { return m_info; }

protected:
	VkBuffer			m_buffer;
	VkBufferCreateInfo	m_info;
	std::shared_ptr<DeviceMemoryManager> m_pMemMgr;
};