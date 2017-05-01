#pragma once
#include "DeviceMemoryManager.h"
#include "MemoryConsumer.h"

class StagingBufferManager;

class Buffer : public DeviceObjectBase, public MemoryConsumer
{
public:
	~Buffer();

	bool Init(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag);

public:
	static std::shared_ptr<Buffer> Create(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag);

public:
	virtual VkBuffer GetDeviceHandle() const { return m_buffer; }
	const VkBufferCreateInfo& GetBufferInfo() const { return m_info; }
	virtual void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes, VkPipelineStageFlagBits dstStage, VkAccessFlags dstAccess);
	virtual uint32_t GetMemoryProperty() const override { return m_memProperty; }
	virtual VkMemoryRequirements GetMemoryReqirments() const override;

protected:
	virtual void BindMemory(VkDeviceMemory memory, uint32_t offset) const override;

protected:
	VkBuffer					m_buffer = 0;
	VkBufferCreateInfo			m_info;
	uint32_t					m_memProperty;

	friend class StagingBufferManager;
};