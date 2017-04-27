#pragma once
#include "DeviceMemoryManager.h"

class StagingBufferManager;

class Buffer : DeviceObjectBase
{
public:
	~Buffer();

	bool Init(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag);

public:
	VkBuffer GetDeviceHandle() const { return m_buffer; }
	const VkBufferCreateInfo& GetBufferInfo() const { return m_info; }
	virtual void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes, VkPipelineStageFlagBits dstStage, VkAccessFlags dstAccess);

protected:
	VkBuffer					m_buffer;
	VkBufferCreateInfo			m_info;
	uint32_t					m_memProperty;

	friend class StagingBufferManager;
};