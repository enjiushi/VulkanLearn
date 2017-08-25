#pragma once
#include "DeviceMemoryManager.h"
#include "MemoryConsumer.h"

class StagingBufferManager;

class Buffer : public DeviceObjectBase<Buffer>
{
public:
	~Buffer();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Buffer>& pSelf, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag);

public:
	static std::shared_ptr<Buffer> Create(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag);

public:
	VkPipelineStageFlags GetAccessStages() const { return m_accessStages; }
	VkAccessFlags GetAccessFlags() const { return m_accessFlags; }
	virtual VkBuffer GetDeviceHandle() const { return m_buffer; }
	const VkBufferCreateInfo& GetBufferInfo() const { return m_info; }
	virtual void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes);
	virtual uint32_t GetMemoryProperty() const { return m_memProperty; }
	virtual VkMemoryRequirements GetMemoryReqirments() const;

protected:
	virtual void BindMemory(VkDeviceMemory memory, uint32_t offset) const;

protected:
	VkBuffer						m_buffer = 0;
	VkBufferCreateInfo				m_info;
	uint32_t						m_memProperty;
	std::shared_ptr<MemoryKey>		m_pMemKey;
	VkPipelineStageFlags			m_accessStages;
	VkAccessFlags					m_accessFlags;

	friend class StagingBufferManager;
	friend class DeviceMemoryManager;
};