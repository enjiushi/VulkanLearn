#include "UniformBuffer.h"

std::vector<VkDescriptorBufferInfo> UniformBuffer::m_uniformBufferTable;

bool UniformBuffer::Init(const std::shared_ptr<Device>& pDevice, uint32_t numBytes)
{
	VkDescriptorBufferInfo info = {};
	info.buffer = GlobalDeviceObjects::GetInstance()->GetBigUniformBuffer()->GetDeviceHandle();
	uint32_t offset = 0;
	uint32_t endByte = 0;
	for (uint32_t i = 0; i < m_uniformBufferTable.size(); i++)
	{
		endByte = offset + numBytes - 1;
		if (endByte < m_uniformBufferTable[i].offset)
		{
			info.offset = offset;
			info.range = numBytes;
			m_uniformBufferTable.insert(m_uniformBufferTable.begin() + i, info);
			m_bufferIndex = i;
			return true;
		}
		else
		{
			offset = m_uniformBufferTable[i].offset + m_uniformBufferTable[i].range;
		}
	}

	if (offset + numBytes > GlobalDeviceObjects::GetInstance()->GetBigUniformBuffer()->GetBufferInfo().size)
		return false;

	info.offset = offset;
	info.range = numBytes;
	m_uniformBufferTable.push_back(info);
	m_bufferIndex = m_uniformBufferTable.size() - 1;
	return true;
}

std::shared_ptr<UniformBuffer> UniformBuffer::Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes)
{
	std::shared_ptr<UniformBuffer> pUniformBuffer = std::make_shared<UniformBuffer>();
	if (pUniformBuffer.get() && pUniformBuffer->Init(pDevice, numBytes))
		return pUniformBuffer;
	return nullptr;
}