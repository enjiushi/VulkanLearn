#pragma once
#include "DeviceObjectBase.h"

class VulkanUtil
{
public:
	static VkAccessFlagBits GetAccessFlagByLayout(VkImageLayout layout);
	static uint32_t GetBytesFromFormat(VkFormat format);
};