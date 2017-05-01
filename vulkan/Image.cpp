#include "Image.h"
#include "GlobalDeviceObjects.h"
#include "DeviceMemoryManager.h"

Image::~Image()
{
	vkDestroyImage(GetDevice()->GetDeviceHandle(), m_image, nullptr);
	GlobalDeviceObjects::GetInstance()->GetDeviceMemMgr()->FreeMemChunk(this);
}

bool Image::Init(const std::shared_ptr<Device>& pDevice, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	CHECK_VK_ERROR(vkCreateImage(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_image));
	GlobalDeviceObjects::GetInstance()->GetDeviceMemMgr()->AllocateMemChunk(this, memoryPropertyFlag);

	m_info = info;
	m_memProperty = memoryPropertyFlag;
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