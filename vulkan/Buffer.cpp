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
	if (!BufferBase::Init(pDevice, pSelf, info))
		return false;
	
	CHECK_VK_ERROR(vkCreateBuffer(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_buffer));
	m_pMemKey = DeviceMemMgr()->AllocateBufferMemChunk(pSelf, memoryPropertyFlag);

	m_isHostVisible = memoryPropertyFlag & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

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
	// If we can directly write data into buffer memory(Probably means that this buffer is host visible), return immediately
	// we can update it directly without staging buffer
	if (DeviceMemMgr()->UpdateBufferMemChunk(m_pMemKey, pData, offset, numBytes))
		return;
	// Otherwise, we have to let staging buffer do its job: copy data to staging buffer manager first, then copy it to device local buffer sometime later
	StagingBufferMgr()->UpdateByteStream(std::static_pointer_cast<Buffer>(GetSelfSharedPtr()), pData, offset, numBytes);
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