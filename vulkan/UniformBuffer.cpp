#include "UniformBuffer.h"

bool UniformBuffer::Init(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	const std::shared_ptr<DeviceMemoryManager>& pMemMgr)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	info.size = numBytes;
	if (!Buffer::Init(pDevice, info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pMemMgr))
		return false;
	return true;
}

std::shared_ptr<UniformBuffer> UniformBuffer::Create(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	const std::shared_ptr<DeviceMemoryManager>& pMemMgr)
{
	std::shared_ptr<UniformBuffer> pIndexBuffer = std::make_shared<UniformBuffer>();
	if (pIndexBuffer.get() && pIndexBuffer->Init(pDevice, numBytes, pMemMgr))
		return pIndexBuffer;
	return nullptr;
}