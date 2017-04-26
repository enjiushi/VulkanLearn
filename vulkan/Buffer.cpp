#include "Buffer.h"
#include "StagingBuffer.h"
#include "StagingBufferManager.h"
#include "GlobalDeviceObjects.h"

Buffer::~Buffer()
{
	vkDestroyBuffer(GetDevice()->GetDeviceHandle(), m_buffer, nullptr);
	GlobalDeviceObjects::GetInstance()->GetDeviceMemMgr()->FreeMemChunk(this);
}

bool Buffer::Init(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag, const void* pData)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	CHECK_VK_ERROR(vkCreateBuffer(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_buffer));
	GlobalDeviceObjects::GetInstance()->GetDeviceMemMgr()->AllocateMemChunk(this, memoryPropertyFlag, pData);

	m_info = info;
	m_memProperty = memoryPropertyFlag;
	return true;
}

void Buffer::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes)
{
	m_dataDirty = true;
	GlobalDeviceObjects::GetInstance()->GetStagingBufferMgr()->UpdateByteStream(this, pData, offset, numBytes);
}