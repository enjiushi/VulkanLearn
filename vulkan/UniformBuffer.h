#pragma once

#include "Buffer.h"
#include "GlobalDeviceObjects.h"

class UniformBuffer : public Buffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<UniformBuffer>& pSelf,
		uint32_t numBytes);

public:
	VkDescriptorBufferInfo GetDescBufferInfo() const 
	{
		return m_uniformBufferTable[m_bufferIndex];
	}

	VkBuffer GetDeviceHandle() const { return GlobalDeviceObjects::GetInstance()->GetBigUniformBuffer()->GetDeviceHandle(); }

public:
	static std::shared_ptr<UniformBuffer> Create(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes);

protected:
	uint32_t m_bufferIndex;
	static std::vector<VkDescriptorBufferInfo> m_uniformBufferTable;
};