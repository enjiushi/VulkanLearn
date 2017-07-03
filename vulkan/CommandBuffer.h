#pragma once

#include "DeviceObjectBase.h"

class CommandPool;

class CommandBuffer : public DeviceObjectBase
{
public:
	~CommandBuffer();

	bool Init(const std::shared_ptr<Device>& pDevice, const VkCommandBufferAllocateInfo& info);

public:
	VkCommandBuffer GetDeviceHandle() const { return m_commandBuffer; }
	VkCommandBufferAllocateInfo GetAllocateInfo() const { return m_info; }

public:
	static std::shared_ptr<CommandBuffer> Create(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<CommandPool>& pCmdPool, VkCommandBufferLevel cmdBufferLevel);

protected:
	VkCommandBuffer					m_commandBuffer;
	VkCommandBufferAllocateInfo		m_info;

	std::shared_ptr<CommandPool>	m_pCommandPool;
};