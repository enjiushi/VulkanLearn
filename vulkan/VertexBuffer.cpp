#include "VertexBuffer.h"

bool VertexBuffer::Init(const std::shared_ptr<Device>& pDevice, 
	uint32_t numBytes,
	const VkVertexInputBindingDescription& bindingDesc,
	const std::vector<VkVertexInputAttributeDescription>& attribDesc,
	const std::shared_ptr<DeviceMemoryManager>& pMemMgr)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	info.size = numBytes;
	if (!Buffer::Init(pDevice, info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pMemMgr))
		return false;

	m_bindingDesc = bindingDesc;
	m_attribDesc = attribDesc;
	m_numVertices = info.size / bindingDesc.stride;
	return true;
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(const std::shared_ptr<Device>& pDevice, 
	uint32_t numBytes,
	const VkVertexInputBindingDescription& bindingDesc,
	const std::vector<VkVertexInputAttributeDescription>& attribDesc,
	const std::shared_ptr<DeviceMemoryManager>& pMemMgr)
{
	std::shared_ptr<VertexBuffer> pVertexBuffer = std::make_shared<VertexBuffer>();
	if (pVertexBuffer.get() && pVertexBuffer->Init(pDevice, numBytes, bindingDesc, attribDesc, pMemMgr))
		return pVertexBuffer;
	return nullptr;
}