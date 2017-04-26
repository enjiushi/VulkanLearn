#pragma once

#include "Buffer.h"

class VertexBuffer : public Buffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		uint32_t numBytes,
		const VkVertexInputBindingDescription& bindingDesc,
		const std::vector<VkVertexInputAttributeDescription>& attribDesc);

public:
	const VkVertexInputBindingDescription& GetBindingDesc() const { return m_bindingDesc; }
	const std::vector<VkVertexInputAttributeDescription>& GetAttribDesc() const { return m_attribDesc; }
	uint32_t GetNumVertices() const { return m_numVertices; }

public:
	static std::shared_ptr<VertexBuffer> Create(const std::shared_ptr<Device>& pDevice, 
		uint32_t numBytes,
		const VkVertexInputBindingDescription& bindingDesc,
		const std::vector<VkVertexInputAttributeDescription>& attribDesc);

protected:
	VkVertexInputBindingDescription					m_bindingDesc;
	std::vector<VkVertexInputAttributeDescription>	m_attribDesc;
	uint32_t										m_numVertices;
};