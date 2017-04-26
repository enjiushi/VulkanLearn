#pragma once

#include "DeviceObjectBase.h"

class CommandPool : public DeviceObjectBase
{
public:
	~CommandPool();

	bool Init(const std::shared_ptr<Device>& pDevice, const VkCommandPoolCreateInfo& info);

public:
	// Temp function
	VkCommandBuffer AllocateCommandBuffer();

public:
	static std::shared_ptr<CommandPool> Create(const std::shared_ptr<Device>& pDevice, const VkCommandPoolCreateInfo& info);

protected:
	VkCommandPool			m_commandPool;
	VkCommandPoolCreateInfo m_info;
};