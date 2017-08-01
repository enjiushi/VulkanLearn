#pragma once

#include "DeviceObjectBase.h"

class SwapChain;
class MemoryKey;

class Image : public DeviceObjectBase<Image>
{
public:
	~Image();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag);

public:
	VkImage GetDeviceHandle() const { return m_image; }
	const VkImageCreateInfo& GetImageInfo() const { return m_info; }
	virtual void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes, VkPipelineStageFlagBits dstStage, VkAccessFlags dstAccess);
	virtual uint32_t GetMemoryProperty() const { return m_memProperty; }
	virtual VkMemoryRequirements GetMemoryReqirments() const;
	virtual VkImageViewCreateInfo GetViewInfo() const { return m_viewInfo; }
	virtual VkImageView GetViewDeviceHandle() const { return m_view; }
	virtual void EnsureImageLayout() = 0;

protected:
	virtual void BindMemory(VkDeviceMemory memory, uint32_t offset) const;
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, VkImage img);
	virtual void CreateImageView(VkImageView* pView, VkImageViewCreateInfo& viewCreateInfo) = 0;

protected:
	VkImage						m_image;
	VkImageCreateInfo			m_info;

	bool						m_shouldDestoryRawImage = true;

	// Put image view here, maybe for now, since I don't need one image with multiple view at this moment
	VkImageViewCreateInfo		m_viewInfo;
	VkImageView					m_view;

	uint32_t					m_memProperty;

	std::shared_ptr<MemoryKey>	m_pMemKey;

	friend class DeviceMemoryManager;
};