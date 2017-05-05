#pragma once

#include "DeviceObjectBase.h"
#include "MemoryConsumer.h"

class SwapChain;

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
	virtual VkImageViewCreateInfo GetViewInfo() const { return m_viewInfo; }
	virtual VkImageView GetViewDeviceHandle() const { return m_view; }
	virtual bool BufferOrImage() const override { return false; }

protected:
	virtual void BindMemory(VkDeviceMemory memory, uint32_t offset) const override;
	bool Init(const std::shared_ptr<Device>& pDevice, VkImage img);

protected:
	VkImage						m_image;
	VkImageCreateInfo			m_info;

	// Put image view here, maybe for now, since I don't need one image with multiple view at this moment
	VkImageViewCreateInfo		m_viewInfo;
	VkImageView					m_view;

	uint32_t					m_memProperty;
};