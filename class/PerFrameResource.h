#pragma once

#include "../vulkan/DeviceObjectBase.h"
#include "../vulkan/CommandPool.h"
#include "../vulkan/CommandBuffer.h"

class DescriptorSet;
class DescriptorPool;
class Fence;
class DescriptorSetLayout;

class PerFrameResource : public DeviceObjectBase<PerFrameResource>
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex, const std::shared_ptr<PerFrameResource>& pSelf);

public:
	static std::shared_ptr<PerFrameResource> Create(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex);

public:
	std::shared_ptr<CommandBuffer> AllocateCommandBuffer
	(
		PhysicalDevice::QueueFamily queueFamily,
		CommandPool::CBPersistancy persistancy, 
		CommandBuffer::CBLevel level
	);
	uint32_t GetFrameIndex() const { return m_frameIndex; }

private:
	std::shared_ptr<CommandPool>		m_commandPools[(uint32_t)PhysicalDevice::QueueFamily::COUNT][(uint32_t)CommandPool::CBPersistancy::COUNT];
	uint32_t							m_frameIndex;
};