#pragma once

#include "DeviceObjectBase.h"

class CommandBuffer;
class Fence;
class PerFrameResource;

class CommandPool : public DeviceObjectBase<CommandPool>
{
public:
	~CommandPool();

	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t queueFamilyIndex, const std::shared_ptr<PerFrameResource>& pPerFrameRes, const std::shared_ptr<CommandPool>& pSelf, VkCommandPoolCreateFlags flags = 0);

public:
	std::shared_ptr<CommandBuffer> AllocatePrimaryCommandBuffer();
	std::shared_ptr<CommandBuffer> AllocateSecondaryCommandBuffer();
	std::vector<std::shared_ptr<CommandBuffer>> AllocatePrimaryCommandBuffers(uint32_t count);
	std::vector<std::shared_ptr<CommandBuffer>> AllocateSecondaryCommandBuffers(uint32_t count);
	VkCommandPool GetDeviceHandle() const { return m_commandPool; }
	void Reset(const std::shared_ptr<Fence>& pFence);
	std::weak_ptr<PerFrameResource> GetPerFrameResource() const { return m_pPerFrameRes; }
	const VkCommandPoolCreateInfo& GetInfo() const { return m_info; }

public:
	static std::shared_ptr<CommandPool> Create(const std::shared_ptr<Device>& pDevice, uint32_t queueFamilyIndex);
	static std::shared_ptr<CommandPool> Create(const std::shared_ptr<Device>& pDevice, uint32_t queueFamilyIndex, const std::shared_ptr<PerFrameResource>& pPerFrameRes);
	static std::shared_ptr<CommandPool> CreateTransientCBPool(const std::shared_ptr<Device>& pDevice, uint32_t queueFamilyIndex, const std::shared_ptr<PerFrameResource>& pPerFrameRes);

protected:
	VkCommandPool					m_commandPool;
	VkCommandPoolCreateInfo			m_info;
	std::weak_ptr<PerFrameResource> m_pPerFrameRes;
};