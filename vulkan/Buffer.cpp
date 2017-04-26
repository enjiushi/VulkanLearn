#include "Buffer.h"

Buffer::~Buffer()
{
	vkDestroyBuffer(GetDevice()->GetDeviceHandle(), m_buffer, nullptr);
	DeviceMemoryManager::GetInstance()->FreeMemChunk(this);
}

bool Buffer::Init(const std::shared_ptr<Device>& pDevice, const VkBufferCreateInfo& info, uint32_t memoryPropertyFlag, const void* pData)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	CHECK_VK_ERROR(vkCreateBuffer(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_buffer));
	DeviceMemoryManager::GetInstance()->AllocateMemChunk(this, memoryPropertyFlag, pData);

	m_info = info;
	return true;
}