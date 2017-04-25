#include "VertexBuffer.h"

bool VertexBuffer::Init(const std::shared_ptr<Device>& pDevice, 
	const VkBufferCreateInfo& info, 
	const VkVertexInputBindingDescription& bindingDesc,
	const std::vector<VkVertexInputAttributeDescription>& attribDesc,
	const std::shared_ptr<DeviceMemoryManager>& pMemMgr)
{
	if (!Buffer::Init(pDevice, info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pMemMgr))
		return false;

	m_bindingDesc = bindingDesc;
	m_attribDesc = attribDesc;
	return true;
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(const std::shared_ptr<Device>& pDevice, 
	const VkBufferCreateInfo& info, 
	const VkVertexInputBindingDescription& bindingDesc,
	const std::vector<VkVertexInputAttributeDescription>& attribDesc,
	const std::shared_ptr<DeviceMemoryManager>& pMemMgr)
{
	std::shared_ptr<VertexBuffer> pVertexBuffer = std::make_shared<VertexBuffer>();
	if (pVertexBuffer.get() && pVertexBuffer->Init(pDevice, info, bindingDesc, attribDesc, pMemMgr))
		return pVertexBuffer;
	return nullptr;
}