#pragma once

#include "../vulkan/VKGPUSyncRes.h"
#include "../vulkan/Device.h"
#include "../vulkan/CommandPool.h"
#include <map>

class ResourceBarrierScheduler : public SelfRefBase<ResourceBarrierScheduler>
{
public:
	static std::shared_ptr<ResourceBarrierScheduler> Create();

protected:
	bool Init(const std::shared_ptr<ResourceBarrierScheduler>& pSchedueler);

public:
	void ClaimResourceUsage
	(
		const std::shared_ptr<CommandBuffer>& pCmdBuffer,
		const std::shared_ptr<VKGPUSyncRes>& pResource,
		VkPipelineStageFlags pipelineStageFlags,
		VkImageLayout imageLayout,
		VkAccessFlags accessFlags
	);

	void ReleaseQueueOwnership
	(
		const std::shared_ptr<CommandBuffer>& pCmdBuffer,
		const std::shared_ptr<VKGPUSyncRes>& pResource,
		PhysicalDevice::QueueFamily	srcQueueFamily,
		VkPipelineStageFlags srcPipelineStageFlags,
		VkImageLayout srcImageLayout,
		VkAccessFlags srcAccessFlags,
		PhysicalDevice::QueueFamily	dstQueueFamily
	);

	void AcquireQueueOwnership
	(
		const std::shared_ptr<CommandBuffer>& pCmdBuffer,
		const std::shared_ptr<VKGPUSyncRes>& pResource,
		PhysicalDevice::QueueFamily	srcQueueFamily,
		PhysicalDevice::QueueFamily	dstQueueFamily,
		VkPipelineStageFlags dstPipelineStageFlags,
		VkImageLayout dstImageLayout,
		VkAccessFlags dstAccessFlags
	);

protected:
	static bool IsAccessWrite(VkAccessFlags accessFlags);

private:
	typedef struct
	{
		VkPipelineStageFlags		stagesFlags;
		VkAccessFlags				accessFlags;
		VkImageLayout				imageLayout;
		bool						isAccessWrite;
		uint32_t					lastWriteIndex;

		// If there're one or multiple continuous read accesses, 
		// accumulate stages here, to make execution dependency
		VkPipelineStageFlags		accumulatedReadStages = 0;
	}ClaimedResourceUsage;

	typedef std::vector<ClaimedResourceUsage> ResourceUsageRecord;		// Claimed resources usage in one pass(draw/dispatch/blit/copy)

	std::map<std::shared_ptr<Base>, ResourceUsageRecord>	m_claimedResourceUsageList;		// Stores all resource usages for all passes within this container
};