#pragma once

#include "DeviceObjectBase.h"

class CommandBuffer;
class CommandPool;
class DescriptorSet;
class DescriptorPool;
class Fence;
class DescriptorSetLayout;

class PerFrameResource : public DeviceObjectBase<PerFrameResource>
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex, const std::shared_ptr<PerFrameResource>& pSelf, bool transientCBPool);

public:
	static std::shared_ptr<PerFrameResource> Create(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex, bool transientCBPool = false);

public:
	std::shared_ptr<CommandBuffer> AllocatePrimaryCommandBuffer();
	std::shared_ptr<CommandBuffer> AllocateSecondaryCommandBuffer();
	std::shared_ptr<DescriptorSet> AllocateDescriptorSet(const std::shared_ptr<DescriptorSetLayout>& pDsLayout);
	uint32_t GetFrameIndex() const { return m_frameIndex; }

private:
	std::shared_ptr<CommandPool>		m_pCommandPool;
	std::shared_ptr<DescriptorPool>		m_pDescriptorPool;
	uint32_t							m_frameIndex;
};