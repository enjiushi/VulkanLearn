#pragma once

#include "DeviceObjectBase.h"
#include "CommandBuffer.h"

class CommandBuffer;
class Fence;
class PerFrameResource;

class CommandPool : public DeviceObjectBase<CommandPool>
{
public:
	enum class CBPersistancy
	{
		PERSISTANT,
		TRANSIENT,
		COUNT
	};

public:
	~CommandPool();

	bool Init
	(
		const std::shared_ptr<Device>& pDevice, 
		PhysicalDevice::QueueFamily queueFamily, 
		CBPersistancy persistancy,
		const std::shared_ptr<CommandPool>& pSelf
	);

public:
	std::shared_ptr<CommandBuffer> AllocateCommandBuffer(CommandBuffer::CBLevel level);
	std::vector<std::shared_ptr<CommandBuffer>> AllocateCommandBuffers(CommandBuffer::CBLevel level, uint32_t count);
	VkCommandPool GetDeviceHandle() const { return m_commandPool; }
	void Reset(const std::shared_ptr<Fence>& pFence);
	const VkCommandPoolCreateInfo& GetInfo() const { return m_info; }

public:
	static std::shared_ptr<CommandPool> Create
	(
		const std::shared_ptr<Device>& pDevice, 
		PhysicalDevice::QueueFamily queueFamily, 
		CBPersistancy persistancy
	);
	static std::shared_ptr<CommandPool> Create
	(
		const std::shared_ptr<Device>& pDevice, 
		PhysicalDevice::QueueFamily queueFamily, 
		CBPersistancy persistancy, 
		const std::shared_ptr<PerFrameResource>& pPerFrameRes
	);

protected:
	VkCommandPool					m_commandPool;
	VkCommandPoolCreateInfo			m_info;
};