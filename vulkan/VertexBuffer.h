#pragma once

#include "Buffer.h"

class VertexBuffer : public Buffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice,
		const std::shared_ptr<VertexBuffer>& pSelf,
		uint32_t numBytes,
		const VkVertexInputBindingDescription& bindingDesc,
		const std::vector<VkVertexInputAttributeDescription>& attribDesc,
		uint32_t vertexFormat);

public:
	const VkVertexInputBindingDescription& GetBindingDesc() const { return m_bindingDesc; }
	const std::vector<VkVertexInputAttributeDescription>& GetAttribDesc() const { return m_attribDesc; }
	uint32_t GetNumVertices() const { return m_numVertices; }
	uint32_t GetVertexFormat() const { return m_vertexFormat; }

public:
	static std::shared_ptr<VertexBuffer> Create(const std::shared_ptr<Device>& pDevice, 
		uint32_t numBytes,
		const VkVertexInputBindingDescription& bindingDesc,
		const std::vector<VkVertexInputAttributeDescription>& attribDesc,
		uint32_t vertexFormat);

protected:
	VkVertexInputBindingDescription					m_bindingDesc;
	std::vector<VkVertexInputAttributeDescription>	m_attribDesc;
	uint32_t										m_numVertices;
	uint32_t										m_vertexFormat;
};