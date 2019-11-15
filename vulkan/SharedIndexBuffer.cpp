#include "SharedIndexBuffer.h"

bool SharedIndexBuffer::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<SharedIndexBuffer>& pSelf,
	uint32_t numBytes,
	VkIndexType type)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	info.size = numBytes;

	if (!SharedBuffer::Init(pDevice, pSelf, info))
		return false;

	m_type = type;

	switch (m_type)
	{
	case VK_INDEX_TYPE_UINT16:
		m_count = numBytes / sizeof(uint16_t);
		break;
	case VK_INDEX_TYPE_UINT32:
		m_count = numBytes / sizeof(uint32_t);
		break;
	default:
		assert(false);
		break;
	}

	m_accessStages = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	m_accessFlags = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

	return true;
}

std::shared_ptr<SharedIndexBuffer> SharedIndexBuffer::Create(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	VkIndexType type)
{
	std::shared_ptr<SharedIndexBuffer> pIndexBuffer = std::make_shared<SharedIndexBuffer>();
	if (pIndexBuffer.get() && pIndexBuffer->Init(pDevice, pIndexBuffer, numBytes, type))
		return pIndexBuffer;
	return nullptr;
}

std::shared_ptr<BufferKey>	SharedIndexBuffer::AcquireBuffer(uint32_t numBytes)
{
	return IndexBufferMgr()->AllocateBuffer(numBytes);

}