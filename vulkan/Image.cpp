#include "Image.h"
#include "GlobalDeviceObjects.h"
#include "DeviceMemoryManager.h"
#include "SwapChain.h"
#include "StagingBuffer.h"
#include "CommandPool.h"
#include "CommandBuffer.h"

Image::~Image()
{
	vkDestroyImageView(GetDevice()->GetDeviceHandle(), m_view, nullptr);
	if (m_shouldDestoryRawImage)
		vkDestroyImage(GetDevice()->GetDeviceHandle(), m_image, nullptr);
}

bool Image::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	// Check if physical device supports input format along with its tiling feature
	VkFormatProperties formatProp = pDevice->GetPhysicalDevice()->GetPhysicalDeviceFormatProperties(info.format);
	if (info.tiling == VK_IMAGE_TILING_LINEAR && (formatProp.linearTilingFeatures & info.usage))
		return false;
	if (info.tiling == VK_IMAGE_TILING_OPTIMAL && (formatProp.optimalTilingFeatures & info.usage != info.usage))
		return false;

	CHECK_VK_ERROR(vkCreateImage(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_image));
	m_pMemKey = DeviceMemMgr()->AllocateImageMemChunk(GetSelfSharedPtr(), memoryPropertyFlag);

	m_info = info;
	m_memProperty = memoryPropertyFlag;

	CreateImageView(&m_view, m_viewInfo);
	return true;
}

bool Image::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, VkImage img)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_image = img;

	CreateImageView(&m_view, m_viewInfo);
	return true;
}

VkMemoryRequirements Image::GetMemoryReqirments() const
{
	VkMemoryRequirements reqs;
	vkGetImageMemoryRequirements(GetDevice()->GetDeviceHandle(), GetDeviceHandle(), &reqs);
	return reqs;
}

void Image::BindMemory(VkDeviceMemory memory, uint32_t offset) const
{
	CHECK_VK_ERROR(vkBindImageMemory(GetDevice()->GetDeviceHandle(), GetDeviceHandle(), memory, offset));
}

void Image::EnsureImageLayout()
{

}