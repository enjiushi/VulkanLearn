#include "SharedVertexBuffer.h"

bool SharedVertexBuffer::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<SharedVertexBuffer>& pSelf,
	uint32_t numBytes,
	const VkVertexInputBindingDescription& bindingDesc,
	const std::vector<VkVertexInputAttributeDescription>& attribDesc,
	uint32_t vertexFormat)
{
	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	m_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	m_info.size = numBytes;

	m_vertexFormat = vertexFormat;

	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_pBufferKey = VertexAttribBufferMgr(m_vertexFormat)->AllocateBuffer(numBytes);
	if (!m_pBufferKey.get())
		return false;

	m_bindingDesc = bindingDesc;
	m_attribDesc = attribDesc;
	m_numVertices = m_info.size / bindingDesc.stride;

	m_accessStages = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	m_accessFlags = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	return true;
}

std::shared_ptr<SharedVertexBuffer> SharedVertexBuffer::Create(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	const VkVertexInputBindingDescription& bindingDesc,
	const std::vector<VkVertexInputAttributeDescription>& attribDesc,
	uint32_t vertexFormat)
{
	std::shared_ptr<SharedVertexBuffer> pVertexBuffer = std::make_shared<SharedVertexBuffer>();
	if (pVertexBuffer.get() && pVertexBuffer->Init(pDevice, pVertexBuffer, numBytes, bindingDesc, attribDesc, vertexFormat))
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