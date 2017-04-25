#include "Buffer.h"

Buffer::~Buffer()
{
	vkDestroyBuffer(GetDevice()->GetDeviceHandle(), m_buffer, nullptr);
	m_pMemMgr->FreeMemChunk(GetDeviceHandle());
}

bool Buffer::Init(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag, const std::shared_ptr<DeviceMemoryManager>& pMemMgr, const void* pData)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	CHECK_VK_ERROR(vkCreateBuffer(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_buffer));
	pMemMgr->AllocateMemChunk(m_buffer, memoryPropertyFlag, pData);

	m_info = info;
	m_pMemMgr = pMemMgr;
	return true;
}