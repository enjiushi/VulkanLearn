#pragma once

#include "../vulkan/DeviceObjectBase.h"

class CommandBuffer;
class CommandPool;
class DescriptorSet;
class DescriptorPool;
class Fence;
class DescriptorSetLayout;

class PerFrameResource : public DeviceObjectBase<PerFrameResource>
{
public:
	enum class QueueFamily
	{
		GRAPHIC,
		COMPUTE,
		TRASFER,
		COUNT
	};

	enum class CBPersistancy
	{
		PERSISTANT,
		TRANSIENT,
		COUNT
	};

	enum class CBLevel
	{
		PRIMARY,
		SECONDARY,
		COUNT
	};

public:
	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex, const std::shared_ptr<PerFrameResource>& pSelf);

public:
	static std::shared_ptr<PerFrameResource> Create(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex);

public:
	std::shared_ptr<CommandBuffer> AllocateCommandBuffer(QueueFamily queueFamily, CBPersistancy persistancy, CBLevel level);
	uint32_t GetFrameIndex() const { return m_frameIndex; }

private:
	std::shared_ptr<CommandPool>		m_commandPools[(uint32_t)QueueFamily::COUNT][(uint32_t)CBPersistancy::COUNT];
	uint32_t							m_frameIndex;
};