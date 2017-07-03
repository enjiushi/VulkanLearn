#pragma once

#include "DeviceObjectBase.h"

class CommandBuffer;

class CommandPool : public DeviceObjectBase
{
public:
	~CommandPool();

	bool Init(const std::shared_ptr<Device>& pDevice, const VkCommandPoolCreateInfo& info);

public:
	static std::shared_ptr<CommandBuffer> AllocatePrimaryCommandBuffer(const std::shared_ptr<CommandPool>& pCmdPool);
	VkCommandPool GetDeviceHandle() const { return m_commandPool; }

public:
	static std::shared_ptr<CommandPool> Create(const std::shared_ptr<Device>& pDevice, const VkCommandPoolCreateInfo& info);

protected:
	VkCommandPool			m_commandPool;
	VkCommandPoolCreateInfo m_info;
};