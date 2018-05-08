#include "VertexBuffer.h"
#include "../common/Util.h"

bool VertexBuffer::Init(const std::shared_ptr<Device>& pDevice, 
	const std::shared_ptr<VertexBuffer>& pSelf,
	uint32_t numBytes,
	uint32_t vertexFormat)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	info.size = numBytes;
	if (!Buffer::Init(pDevice, pSelf, info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	InitDesc(numBytes, vertexFormat);

	return true;
}

void VertexBuffer::InitDesc(uint32_t numBytes, uint32_t vertexFormat)
{
	uint32_t vertexBytes = GetVertexBytes(vertexFormat);

	m_numVertices = numBytes / vertexBytes;

	m_accessStages = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	m_accessFlags = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

	m_vertexFormat = vertexFormat;
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(const std::shared_ptr<Device>& pDevice, 
	uint32_t numBytes,
	uint32_t vertexFormat)
{
	std::shared_ptr<VertexBuffer> pVertexBuffer = std::make_shared<VertexBuffer>();
	if (pVertexBuffer.get() && pVertexBuffer->Init(pDevice, pVertexBuffer, numBytes, vertexFormat))
		return pVertexBuffer;
	return nullptr;
}