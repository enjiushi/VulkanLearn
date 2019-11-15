#pragma once
#include "DeviceMemoryManager.h"
#include "MemoryConsumer.h"
#include "BufferBase.h"

class StagingBufferManager;

class Buffer : public BufferBase
{
public:
	virtual ~Buffer();

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Buffer>& pSelf, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag);

public:
	static std::shared_ptr<Buffer> Create(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag);

public:
	virtual VkMemoryRequirements GetMemoryReqirments() const;

	uint32_t GetBufferOffset() const override { return 0; }
	bool IsHostVisible() const override { return m_isHostVisible; }
	VkBuffer GetDeviceHandle() const override { return m_buffer; }
	void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes) override;

protected:
	void BindMemory(VkDeviceMemory memory, uint32_t offset) const;

protected:
	VkBuffer						m_buffer = 0;
	std::shared_ptr<MemoryKey>		m_pMemKey;
	bool							m_isHostVisible;

	friend class StagingBufferManager;
	friend class DeviceMemoryManager;
};