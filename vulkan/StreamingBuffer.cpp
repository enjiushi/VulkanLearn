#include "StreamingBuffer.h"
#include "GlobalDeviceObjects.h"

bool StreamingBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<StreamingBuffer>& pSelf, uint32_t numBytes)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	info.size = numBytes;

	if (!SharedBuffer::Init(pDevice, pSelf, info))
		return false;

	m_accessStages = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
		| VK_PIPELINE_STAGE_TRANSFER_BIT;
	m_accessFlags = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
		| VK_ACCESS_TRANSFER_READ_BIT;

	return true;
}

std::shared_ptr<StreamingBuffer> StreamingBuffer::Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes)
{
	std::shared_ptr<StreamingBuffer> pStreamingBuffer = std::make_shared<StreamingBuffer>();
	if (pStreamingBuffer.get() && pStreamingBuffer->Init(pDevice, pStreamingBuffer, numBytes))
		return pStreamingBuffer;
	return nullptr;
}

std::shared_ptr<BufferKey>	StreamingBuffer::AcquireBuffer(uint32_t numBytes)
{
	return StreamingBufferMgr()->AllocateBuffer(numBytes);
}