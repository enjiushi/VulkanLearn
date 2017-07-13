#pragma once

#include "../vulkan/DeviceObjectBase.h"

class CommandBuffer;
class CommandPool;
class DescriptorSet;
class DescriptorPool;
class Fence;
class DescriptorLayout;

class PerFrameData
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice);

public:
	std::shared_ptr<CommandBuffer> AllocateCommandBuffer();
	std::shared_ptr<DescriptorSet> AllocateDescriptorSet(const std::shared_ptr<DescriptorLayout>& pDSLayout);

public:
	static std::shared_ptr<PerFrameData> Create(const std::shared_ptr<Device>& pDevice);

protected:
	void WaitAndResetFence();

private:
	std::shared_ptr<CommandPool>		m_pCommandPool;
	std::shared_ptr<DescriptorPool>		m_pDescriptorPool;
	std::shared_ptr<Fence>				m_pFence;
};