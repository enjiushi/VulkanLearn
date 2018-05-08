#include "SharedVertexBuffer.h"

bool SharedVertexBuffer::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<SharedVertexBuffer>& pSelf,
	uint32_t numBytes,
	uint32_t vertexFormat)
{
	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	m_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	m_info.size = numBytes;

	InitDesc(numBytes, vertexFormat);

	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_pBufferKey = VertexAttribBufferMgr(m_vertexFormat)->AllocateBuffer(numBytes);
	if (!m_pBufferKey.get())
		return false;

	return true;
}

std::shared_ptr<SharedVertexBuffer> SharedVertexBuffer::Create(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	uint32_t vertexFormat)
{
	std::shared_ptr<SharedVertexBuffer> pVertexBuffer = std::make_shared<SharedVertexBuffer>();
	if (pVertexBuffer.get() && pVertexBuffer->Init(pDevice, pVertexBuffer, numBytes, vertexFormat))
		return pVertexBuffer;
	return nullptr;
}

void SharedVertexBuffer::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes)
{
	m_pBufferKey->GetSharedBufferMgr()->UpdateByteStream(pData, GetSelfSharedPtr(), m_pBufferKey, offset, numBytes);
}

uint32_t SharedVertexBuffer::GetBufferOffset() const
{
	return m_pBufferKey->GetSharedBufferMgr()->GetOffset(m_pBufferKey);
}