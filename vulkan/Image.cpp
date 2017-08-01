#include "Image.h"
#include "GlobalDeviceObjects.h"
#include "DeviceMemoryManager.h"
#include "SwapChain.h"

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

	CHECK_VK_ERROR(vkCreateImage(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_image));
	m_pMemKey = DeviceMemMgr()->AllocateImageMemChunk(GetSelfSharedPtr(), memoryPropertyFlag);

	m_info = info;
	m_memProperty = memoryPropertyFlag;
	return true;
}

bool Image::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, VkImage img)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_image = img;

	return true;
}

void Image::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes, VkPipelineStageFlagBits dstStage, VkAccessFlags dstAccess)
{
	//FIXME: do this future
	//GlobalDeviceObjects::GetInstance()->GetStagingBufferMgr()->UpdateByteStream(this, pData, offset, numBytes, dstStage, dstAccess);
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