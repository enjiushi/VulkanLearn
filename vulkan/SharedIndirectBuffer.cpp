#include "SharedIndirectBuffer.h"
#include "SharedBuffer.h"

bool SharedIndirectBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SharedIndirectBuffer>& pSelf, uint32_t numBytes)
{
	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	m_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	m_info.size = numBytes;

	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_pBufferKey = IndirectBufferMgr()->AllocateBuffer(numBytes);
	if (!m_pBufferKey.get())
		return false;

	m_accessStages = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	m_accessFlags = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	return true;
}

std::shared_ptr<SharedIndirectBuffer> SharedIndirectBuffer::Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes)
{
	std::shared_ptr<SharedIndirectBuffer> pIndirectBuffer = std::make_shared<SharedIndirectBuffer>();
	if (pIndirectBuffer.get() && pIndirectBuffer->Init(pDevice, pIndirectBuffer, numBytes))
		return pIndirectBuffer;
	return nullptr;
}

void SharedIndirectBuffer::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes)
{
	m_pBufferKey->GetSharedBufferMgr()->UpdateByteStream(pData, std::static_pointer_cast<SharedBuffer>(GetSelfSharedPtr()), m_pBufferKey, offset, numBytes);
}

uint32_t SharedIndirectBuffer::GetBufferOffset() const
{
	return m_pBufferKey->GetSharedBufferMgr()->GetOffset(m_pBufferKey);
}