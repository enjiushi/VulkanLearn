#include "Buffer.h"
#include "StagingBuffer.h"
#include "StagingBufferManager.h"
#include "GlobalDeviceObjects.h"

Buffer::~Buffer()
{
	if (m_buffer)
		vkDestroyBuffer(GetDevice()->GetDeviceHandle(), m_buffer, nullptr);
	GlobalDeviceObjects::GetInstance()->GetDeviceMemMgr()->FreeMemChunk(this);
}

bool Buffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Buffer>& pSelf, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	CHECK_VK_ERROR(vkCreateBuffer(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_buffer));
	GlobalDeviceObjects::GetInstance()->GetDeviceMemMgr()->AllocateMemChunk(this, memoryPropertyFlag);

	m_info = info;
	m_memProperty = memoryPropertyFlag;
	return true;
}

std::shared_ptr<Buffer> Buffer::Create(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag)
{
	std::shared_ptr<Buffer> pBuffer = std::make_shared<Buffer>();
	if (pBuffer.get() && pBuffer->Init(pDevice, pBuffer, info, memoryPropertyFlag))
		return pBuffer;
	return nullptr;
}

void Buffer::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes, VkPipelineStageFlagBits dstStage, VkAccessFlags dstAccess)
{
	GlobalDeviceObjects::GetInstance()->GetStagingBufferMgr()->UpdateByteStream(GetSelfSharedPtr(), pData, offset, numBytes, dstStage, dstAccess);
}

VkMemoryRequirements Buffer::GetMemoryReqirments() const
{
	VkMemoryRequirements reqs;
	vkGetBufferMemoryRequirements(GetDevice()->GetDeviceHandle(), GetDeviceHandle(), &reqs);
	return reqs;
}

void Buffer::BindMemory(VkDeviceMemory memory, uint32_t offset) const
{
	CHECK_VK_ERROR(vkBindBufferMemory(GetDevice()->GetDeviceHandle(), GetDeviceHandle(), memory, offset));
}