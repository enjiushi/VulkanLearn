#pragma once

#include "DeviceObjectBase.h"
#include "MemoryConsumer.h"

class Image : public DeviceObjectBase, public MemoryConsumer
{
public:
	~Image();

	bool Init(const std::shared_ptr<Device>& pDevice, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag);

public:
	VkImage GetDeviceHandle() const { return m_image; }
	const VkImageCreateInfo& GetImageInfo() const { return m_info; }
	virtual void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes, VkPipelineStageFlagBits dstStage, VkAccessFlags dstAccess) override;
	virtual uint32_t GetMemoryProperty() const override { return m_memProperty; }
	virtual VkMemoryRequirements GetMemoryReqirments() const override;

protected:
	virtual void BindMemory(VkDeviceMemory memory, uint32_t offset) const override;

protected:
	VkImage						m_image;
	VkImageCreateInfo			m_info;
	uint32_t					m_memProperty;
};