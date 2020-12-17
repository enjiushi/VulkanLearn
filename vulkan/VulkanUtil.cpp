#include "VulkanUtil.h"

VkAccessFlagBits VulkanUtil::GetAccessFlagByLayout(VkImageLayout layout)
{
	if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		return VK_ACCESS_SHADER_READ_BIT;
	if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if (layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// FIXME: supports these for now, more to add
	ASSERTION(false);
	return (VkAccessFlagBits)0;
}

uint32_t VulkanUtil::GetBytesFromFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_R8G8B8A8_UNORM:				return 4;
	case VK_FORMAT_R8_UNORM:					return 1;
	case VK_FORMAT_R16G16B16A16_SFLOAT:			return 8;
	case VK_FORMAT_R16_SFLOAT:					return 2;
	case VK_FORMAT_R32_SFLOAT:					return 4;
	case VK_FORMAT_R16G16_SFLOAT:				return 4;
	case VK_FORMAT_D24_UNORM_S8_UINT:			return 4;
	case VK_FORMAT_D32_SFLOAT_S8_UINT:			return 6;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:	return 4;
	case VK_FORMAT_D32_SFLOAT:					return 4;
	case VK_FORMAT_R16G16_SNORM:				return 4;
	case VK_FORMAT_R32G32B32_SFLOAT:			return 12;
	case VK_FORMAT_R32G32B32A32_SFLOAT:			return 16;
	default: ASSERTION(false);	// New one used, add it here
	}
	return 0;
}