#pragma once

#include "DeviceObjectBase.h"

class CommandBuffer;
class CommandPool;
class DescriptorSet;
class DescriptorPool;
class Fence;
class DescriptorLayout;

class PerFrameResource
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex);

public:
	static std::shared_ptr<PerFrameResource> Create(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex);

private:
	std::shared_ptr<CommandPool>		m_pCommandPool;
	std::shared_ptr<DescriptorPool>		m_pDescriptorPool;
	uint32_t							m_frameIndex;
};