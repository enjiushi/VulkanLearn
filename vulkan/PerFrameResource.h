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
	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex, const std::shared_ptr<PerFrameResource>& pSelf);

public:
	static std::shared_ptr<PerFrameResource> Create(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex);

public:
	std::shared_ptr<CommandBuffer> AllocatePersistantPrimaryCommandBuffer();
	std::shared_ptr<CommandBuffer> AllocatePersistantSecondaryCommandBuffer();
	std::shared_ptr<CommandBuffer> AllocateTransientPrimaryCommandBuffer();
	std::shared_ptr<CommandBuffer> AllocateTransientSecondaryCommandBuffer();
	std::shared_ptr<DescriptorSet> AllocateDescriptorSet(const std::shared_ptr<DescriptorSetLayout>& pDsLayout);
	uint32_t GetFrameIndex() const { return m_frameIndex; }

private:
	std::shared_ptr<CommandPool>		m_pPersistantCBPool;
	std::shared_ptr<CommandPool>		m_pTransientCBPool;
	std::shared_ptr<DescriptorPool>		m_pDescriptorPool;
	uint32_t							m_frameIndex;
};