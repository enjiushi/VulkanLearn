#pragma once
#include "DeviceMemoryManager.h"

class Buffer : DeviceObjectBase
{
public:
	~Buffer();

	bool Init(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag, const void* pData = nullptr);

public:
	VkBuffer GetDeviceHandle() const { return m_buffer; }
	const VkBufferCreateInfo& GetBufferInfo() const { return m_info; }
	bool IsDataDirty() const { return m_dataDirty; }
	virtual void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes);

protected:
	VkBuffer			m_buffer;
	VkBufferCreateInfo	m_info;
	uint32_t			m_memProperty;
	bool				m_dataDirty = false;
};