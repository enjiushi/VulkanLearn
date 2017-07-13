#pragma once

#include "DeviceObjectBase.h"

class CommandBuffer;
class Fence;

class CommandPool : public DeviceObjectBase<CommandPool>
{
public:
	~CommandPool();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<CommandPool>& pSelf);

public:
	std::shared_ptr<CommandBuffer> AllocatePrimaryCommandBuffer();
	std::vector<std::shared_ptr<CommandBuffer>> AllocatePrimaryCommandBuffers(uint32_t count);
	VkCommandPool GetDeviceHandle() const { return m_commandPool; }
	void Reset(const std::shared_ptr<Fence>& pFence);

public:
	static std::shared_ptr<CommandPool> Create(const std::shared_ptr<Device>& pDevice);

protected:
	VkCommandPool			m_commandPool;
	VkCommandPoolCreateInfo m_info;

	std::vector<std::shared_ptr<CommandBuffer>> m_cmdBufferList;
};