#include "SharedBuffer.h"

bool SharedBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SharedBuffer>& pSelf, const VkBufferCreateInfo& info)
{
	if (!BufferBase::Init(pDevice, pSelf, info))
		return false;
	
	m_pBufferKey = AcquireBuffer((uint32_t)info.size);
	if (!m_pBufferKey.get())
		return false;;

	return true;
}

uint32_t SharedBuffer::GetBufferOffset() const
{
	return m_pBufferKey->GetSharedBufferMgr()->GetOffset(m_pBufferKey);
}

bool SharedBuffer::IsHostVisible() const
{
	return m_pBufferKey->GetSharedBufferMgr()->GetBuffer()->IsHostVisible();
}

VkBuffer SharedBuffer::GetDeviceHandle() const
{
	return m_pBufferKey->GetSharedBufferMgr()->GetDeviceHandle();
}

void SharedBuffer::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes)
{
	m_pBufferKey->GetSharedBufferMgr()->UpdateByteStream(pData, std::static_pointer_cast<SharedBuffer>(GetSelfSharedPtr()), m_pBufferKey, offset, numBytes);
}