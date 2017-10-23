#include "Buffer.h"
#include "StagingBuffer.h"
#include "StagingBufferManager.h"
#include "GlobalDeviceObjects.h"

Buffer::~Buffer()
{
	if (m_buffer)
		vkDestroyBuffer(GetDevice()->GetDeviceHandle(), m_buffer, nullptr);
}

bool Buffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Buffer>& pSelf, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;
	
	CHECK_VK_ERROR(vkCreateBuffer(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_buffer));
	m_pMemKey = DeviceMemMgr()->AllocateBufferMemChunk(GetSelfSharedPtr(), memoryPropertyFlag);

	m_info = info;
	m_memProperty = memoryPropertyFlag;
	m_pData = GetDataPtrInternal();
	return true;
}

std::shared_ptr<Buffer> Buffer::Create(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag)
{
	std::shared_ptr<Buffer> pBuffer = std::make_shared<Buffer>();
	if (pBuffer.get() && pBuffer->Init(pDevice, pBuffer, info, memoryPropertyFlag))
		return pBuffer;
	return nullptr;
}

void Buffer::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes)
{
	// If we have a data pointer to this buffer, we can update it directly without staging buffer
	if (m_pData)
		DeviceMemMgr()->UpdateBufferMemChunk(m_pMemKey, m_memProperty, pData, offset, numBytes);
	// Else, we have to let staging buffer do its job: copy data to staging buffer manager first, then copy it to device local buffer sometime later
	else
		StagingBufferMgr()->UpdateByteStream(GetSelfSharedPtr(), pData, offset, numBytes);
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

void* Buffer::GetDataPtrInternal()
{
	return DeviceMemMgr()->GetDataPtr(m_pMemKey, 0, m_info.size);
}

void* Buffer::GetDataPtr()
{
	return m_pData;
}

void* Buffer::GetDataPtr(uint32_t offset, uint32_t numBytes)
{
	return (char*)m_pData + offset;
}