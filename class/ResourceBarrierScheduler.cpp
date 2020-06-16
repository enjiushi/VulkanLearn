#include "ResourceBarrierScheduler.h"
#include "../vulkan/CommandBuffer.h"

std::shared_ptr<ResourceBarrierScheduler> ResourceBarrierScheduler::Create()
{
	std::shared_ptr<ResourceBarrierScheduler> pScheduler = std::make_shared<ResourceBarrierScheduler>();
	if (pScheduler != nullptr && pScheduler->Init(pScheduler))
		return pScheduler;
	return nullptr;
}

bool ResourceBarrierScheduler::Init(const std::shared_ptr<ResourceBarrierScheduler>& pSchedueler)
{
	if (!SelfRefBase<ResourceBarrierScheduler>::Init(pSchedueler))
		return false;

	return true;
}

bool ResourceBarrierScheduler::IsAccessWrite(VkAccessFlags accessFlags)
{
	return (accessFlags & VK_ACCESS_SHADER_WRITE_BIT) ||
		(accessFlags & VK_ACCESS_MEMORY_WRITE_BIT) ||
		(accessFlags & VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT) ||
		(accessFlags & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT) ||
		(accessFlags & VK_ACCESS_TRANSFER_WRITE_BIT);	// Add more if needed

}

void ResourceBarrierScheduler::ClaimResourceUsage
(
	const std::shared_ptr<CommandBuffer>& pCmdBuffer,
	const std::shared_ptr<VKGPUSyncRes>& pResource,
	VkPipelineStageFlags pipelineStageFlags,
	VkImageLayout imageLayout,
	VkAccessFlags accessFlags
)
{
	ResourceUsageRecord& usageRecord = m_claimedResourceUsageList[pResource];

	ClaimedResourceUsage usage = { pipelineStageFlags, accessFlags, imageLayout, IsAccessWrite(accessFlags), (uint32_t)-1 };

	VkImageLayout srcImageLayout;
	if (usageRecord.size() != 0)
	{
		if (usageRecord[usageRecord.size() - 1].isAccessWrite)
			usage.lastWriteIndex = (uint32_t)usageRecord.size() - 1;
		else
			usage.lastWriteIndex = usageRecord[usageRecord.size() - 1].lastWriteIndex;
		srcImageLayout = usageRecord[usageRecord.size() - 1].imageLayout;
	}
	else
		srcImageLayout = imageLayout;
	usageRecord.push_back(usage);

	VkPipelineStageFlags srcStageFlags = 0;
	VkAccessFlags srcAccessFlags = 0;

	VkPipelineStageFlags dstStageFlags = usage.stagesFlags;
	VkAccessFlags dstAccessFlags = usage.accessFlags;
	VkImageLayout dstImageLayout = imageLayout;

	// If current access isn't a write, accumulate its own stages, for execution dependency
	if (!usageRecord[usageRecord.size() - 1].isAccessWrite)
		usageRecord[usageRecord.size() - 1].accumulatedReadStages = usageRecord[usageRecord.size() - 1].stagesFlags;

	// Accumulate stages for continous read, for execution dependency
	if (usageRecord.size() >= 2 &&
		!usageRecord[usageRecord.size() - 1].isAccessWrite &&
		!usageRecord[usageRecord.size() - 2].isAccessWrite)
		usageRecord[usageRecord.size() - 1].accumulatedReadStages |= usageRecord[usageRecord.size() - 2].accumulatedReadStages;

	// If there's a write before current access
	// We need more than just a simple execution dependency
	if (usageRecord[usageRecord.size() - 1].lastWriteIndex != -1)
	{
		// Add current access stages
		srcStageFlags |= usageRecord[usageRecord[usageRecord.size() - 1].lastWriteIndex].stagesFlags;
		// Add accumulated stages of previous reads
		// It'll be current stages if it's a write, so don't worry
		srcStageFlags |= usageRecord[usageRecord.size() - 2].accumulatedReadStages;
		srcAccessFlags = usageRecord[usageRecord[usageRecord.size() - 1].lastWriteIndex].accessFlags;
	}

	// IF there're stages or access flags missing a flush, or image layout is different
	// We plant a barrier here
	if (srcStageFlags != 0 ||
		srcAccessFlags != 0 ||
		srcImageLayout != dstImageLayout)
	{
		std::vector<VkMemoryBarrier> memBarriers;
		std::vector<VkBufferMemoryBarrier> bufferMemBarriers;
		std::vector<VkImageMemoryBarrier> imageMemBarriers;

		// Don't do anything if barrier is not necessary
		if (srcStageFlags == 0)
			return;

		bool needResourceBarrier = true;
		// Prepare barriers only if previous write is not flushed or layout is different
		if (srcAccessFlags != 0 || srcImageLayout != dstImageLayout)
		{
			if (srcImageLayout == dstImageLayout)
			{
				// To check if there's already a existing barrier for last GPU write
				needResourceBarrier = (usageRecord[usageRecord[usageRecord.size() - 1].lastWriteIndex].flushedStages[dstAccessFlags] & dstStageFlags) == 0;
			}
			// Different layout definitely requires a barrier to change
			else{}
		}

		if (needResourceBarrier)
		{
			pResource->PrepareBarriers
			(
				srcAccessFlags,
				srcImageLayout,
				dstAccessFlags,
				dstImageLayout,
				memBarriers, bufferMemBarriers, imageMemBarriers
			);

			// Add current dst stage to flushed stage list
			usageRecord[usageRecord[usageRecord.size() - 1].lastWriteIndex].flushedStages[dstAccessFlags] |= dstStageFlags;
		}

		// Attach a barrier, with or without mem/buffer/image barriers
		pCmdBuffer->AttachBarriers
		(
			srcStageFlags,
			dstStageFlags,
			memBarriers,
			bufferMemBarriers,
			imageMemBarriers
		);
	}
}

void ResourceBarrierScheduler::ReleaseQueueOwnership
(
	const std::shared_ptr<CommandBuffer>& pCmdBuffer,
	const std::shared_ptr<VKGPUSyncRes>& pResource,
	PhysicalDevice::QueueFamily	srcQueueFamily,
	PhysicalDevice::QueueFamily	dstQueueFamily
)
{
	ResourceUsageRecord& usageRecord = m_claimedResourceUsageList[pResource];

	ClaimedResourceUsage usage;
	usage.lastWriteIndex = -1;

	if (usageRecord.size() != 0)
	{
		if (usageRecord[usageRecord.size() - 1].isAccessWrite)
			usage.lastWriteIndex = (uint32_t)usageRecord.size() - 1;
		else
			usage.lastWriteIndex = usageRecord[usageRecord.size() - 1].lastWriteIndex;
	}

	if (usage.lastWriteIndex != -1)
	{
		std::vector<VkMemoryBarrier> memBarriers;
		std::vector<VkBufferMemoryBarrier> bufferMemBarriers;
		std::vector<VkImageMemoryBarrier> imageMemBarriers;

		pResource->PrepareQueueReleaseBarrier
		(
			usageRecord[usage.lastWriteIndex].accessFlags,
			usageRecord[usage.lastWriteIndex].imageLayout,
			srcQueueFamily,
			dstQueueFamily,
			memBarriers,
			bufferMemBarriers,
			imageMemBarriers
		);

		// Only attach barrier if there's a write for queue release/acquire
		pCmdBuffer->AttachBarriers
		(
			usageRecord[usage.lastWriteIndex].stagesFlags,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			memBarriers,
			bufferMemBarriers,
			imageMemBarriers
		);

		usage.isQueueReleaseIssued = true;
		usage.srcQueueFamily = srcQueueFamily;
		usage.dstQueueFamily = dstQueueFamily;

		usage.imageLayout = usageRecord[usage.lastWriteIndex].imageLayout;

		usageRecord.push_back(usage);
	}
}

void ResourceBarrierScheduler::AcquireQueueOwnership
(
	const std::shared_ptr<CommandBuffer>& pCmdBuffer,
	const std::shared_ptr<VKGPUSyncRes>& pResource,
	PhysicalDevice::QueueFamily	srcQueueFamily,
	PhysicalDevice::QueueFamily	dstQueueFamily,
	VkPipelineStageFlags dstPipelineStageFlags,
	VkImageLayout dstImageLayout,
	VkAccessFlags dstAccessFlags
)
{
	ResourceUsageRecord& usageRecord = m_claimedResourceUsageList[pResource];

	ClaimedResourceUsage usage;
	usage.lastWriteIndex = -1;

	// No need for ownership acquire
	if (usageRecord.size() == 0)
		return;

	// No previous ownership release, then no need for acquire
	if (!usageRecord[usageRecord.size() - 1].isQueueReleaseIssued)
		return;

	ASSERTION
	(
		usageRecord[usageRecord.size() - 1].srcQueueFamily == srcQueueFamily 
		&& usageRecord[usageRecord.size() - 1].dstQueueFamily == dstQueueFamily
		&& usageRecord[usageRecord.size() - 1].imageLayout == dstImageLayout
	);

	std::vector<VkMemoryBarrier> memBarriers;
	std::vector<VkBufferMemoryBarrier> bufferMemBarriers;
	std::vector<VkImageMemoryBarrier> imageMemBarriers;

	pResource->PrepareQueueAcquireBarrier
	(
		dstAccessFlags,
		dstImageLayout,
		srcQueueFamily,
		dstQueueFamily,
		memBarriers,
		bufferMemBarriers,
		imageMemBarriers
	);

	// Only attach barrier if there's a write for queue release/acquire
	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		dstPipelineStageFlags,
		memBarriers,
		bufferMemBarriers,
		imageMemBarriers
	);

	// Clear previous queue usage record for new queue
	usageRecord.clear();
}