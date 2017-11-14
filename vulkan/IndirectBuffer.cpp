#include "IndirectBuffer.h"

bool IndirectBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<IndirectBuffer>& pSelf, uint32_t numBytes)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	info.size = numBytes;
	if (!Buffer::Init(pDevice, pSelf, info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		return false;

	m_accessStages = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	m_accessFlags = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	return true;
}

std::shared_ptr<IndirectBuffer> IndirectBuffer::Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes)
{
	std::shared_ptr<IndirectBuffer> pIndirectBuffer = std::make_shared<IndirectBuffer>();
	if (pIndirectBuffer.get() && pIndirectBuffer->Init(pDevice, pIndirectBuffer, numBytes))
		return pIndirectBuffer;
	return nullptr;
}

void IndirectBuffer::SetIndirectCmd(uint32_t index, const VkDrawIndexedIndirectCommand& cmd)
{
	UpdateByteStream(&cmd, index * sizeof(VkDrawIndexedIndirectCommand), sizeof(VkDrawIndexedIndirectCommand));
}