#include "SharedVertexBuffer.h"
#include "../common/Util.h"

bool SharedVertexBuffer::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<SharedVertexBuffer>& pSelf,
	uint32_t numBytes,
	uint32_t vertexFormat)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	info.size = numBytes;

	m_vertexFormat = vertexFormat;

	if (!SharedBuffer::Init(pDevice, pSelf, info))
		return false;

	m_numVertices = numBytes / GetVertexBytes(vertexFormat);

	m_accessStages = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	m_accessFlags = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

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

std::shared_ptr<BufferKey>	SharedVertexBuffer::AcquireBuffer(uint32_t numBytes)
{
	return VertexAttribBufferMgr(m_vertexFormat)->AllocateBuffer(numBytes);
}