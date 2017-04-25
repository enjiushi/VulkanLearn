#pragma once

#include "Buffer.h"

class VertexBuffer : Buffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		const VkBufferCreateInfo& info, 
		const VkVertexInputBindingDescription& bindingDesc,
		const std::vector<VkVertexInputAttributeDescription>& attribDesc,
		const std::shared_ptr<DeviceMemoryManager>& pMemMgr);

public:
	const VkVertexInputBindingDescription& GetBindingDesc() const { return m_bindingDesc; }
	const std::vector<VkVertexInputAttributeDescription>& GetAttribDesc() const { return m_attribDesc; }

public:
	static std::shared_ptr<VertexBuffer> Create(const std::shared_ptr<Device>& pDevice, 
		const VkBufferCreateInfo& info, 
		const VkVertexInputBindingDescription& bindingDesc,
		const std::vector<VkVertexInputAttributeDescription>& attribDesc,
		const std::shared_ptr<DeviceMemoryManager>& pMemMgr);

protected:
	VkVertexInputBindingDescription					m_bindingDesc;
	std::vector<VkVertexInputAttributeDescription>	m_attribDesc;
};