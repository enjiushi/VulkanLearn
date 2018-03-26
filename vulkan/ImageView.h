#pragma once
#include "DeviceObjectBase.h"

class StagingBufferManager;

class ImageView : public DeviceObjectBase<ImageView>
{
public:
	~ImageView();

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<ImageView>& pSelf, const VkImageViewCreateInfo& info);

public:
	static std::shared_ptr<ImageView> Create(const std::shared_ptr<Device>& pDevice, const VkImageViewCreateInfo& info);

public:
	VkImageView GetDeviceHandle() const { return m_imageView; }

protected:
	VkImageView	m_imageView;
};