#include "SharedIndirectBuffer.h"
#include "GlobalDeviceObjects.h"

bool SharedIndirectBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SharedIndirectBuffer>& pSelf, uint32_t numBytes)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	info.size = numBytes;

	if (!SharedBuffer::Init(pDevice, pSelf, info))
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

std::shared_ptr<BufferKey>	SharedIndirectBuffer::AcquireBuffer(uint32_t numBytes)
{
	return IndirectBufferMgr()->AllocateBuffer(numBytes);
}

void SharedIndirectBuffer::SetIndirectCmd(uint32_t index, const VkDrawIndexedIndirectCommand& cmd)
{
	UpdateByteStream(&cmd, index * sizeof(VkDrawIndexedIndirectCommand), sizeof(VkDrawIndexedIndirectCommand));
}