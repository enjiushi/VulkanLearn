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
	PhysicalDevice::QueueFamily	queueFamily,
	VkPipelineStageFlags pipelineStageFlags,
	VkImageLayout imageLayout,
	VkAccessFlags accessFlags
)
{
	ResourceUsageRecord& usageRecord = m_claimedResourceUsageList[pResource];

	ClaimedResourceUsage usage = { queueFamily, pipelineStageFlags, accessFlags, imageLayout, IsAccessWrite(accessFlags), (uint32_t)-1 };

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

	VkPipelineStageFlags toBeFlushedSrcStageFlags = 0;
	VkAccessFlags toBeFlushedSrcAccessFlags = 0;
	VkPipelineStageFlags toBeFlushedDstStageFlags = 0;
	VkAccessFlags toBeFlushedDstAccessFlags = 0;
	VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (usageRecord.size() != 0)
	{
		uint32_t lastWriteIndex = usage.lastWriteIndex;
		do
		{
			lastWriteIndex = usageRecord[usageRecord.size() - 1].lastWriteIndex;
			if ((usageRecord[lastWriteIndex].flushedStagesFlags & usage.stagesFlags) != usage.stagesFlags ||
				(usageRecord[lastWriteIndex].flushedAccessFlags & usage.accessFlags) != usage.accessFlags)
			{
				toBeFlushedSrcStageFlags = usageRecord[lastWriteIndex].stagesFlags;
				toBeFlushedSrcAccessFlags = usageRecord[lastWriteIndex].accessFlags;

				toBeFlushedDstStageFlags |= (usageRecord[lastWriteIndex].flushedStagesFlags & usage.stagesFlags) ^ usage.stagesFlags;
				toBeFlushedDstAccessFlags |= (usageRecord[lastWriteIndex].flushedAccessFlags & usage.accessFlags) ^ usage.accessFlags;

				usageRecord[lastWriteIndex].flushedStagesFlags |= usage.stagesFlags;
				usageRecord[lastWriteIndex].flushedAccessFlags |= usage.accessFlags;
			}
		} while (lastWriteIndex != -1);
	}

	// IF there're stages or access flags missing a flush, or image layout is different
	// We plant a barrier here
	if (toBeFlushedDstStageFlags != 0 || toBeFlushedDstAccessFlags != 0 || srcImageLayout != dstImageLayout)
	{
		std::vector<VkMemoryBarrier> memBarriers;
		std::vector<VkBufferMemoryBarrier> bufferMemBarriers;
		std::vector<VkImageMemoryBarrier> imageMemBarriers;

		pResource->PrepareBarriers
		(
			toBeFlushedSrcAccessFlags,
			srcImageLayout,
			toBeFlushedDstAccessFlags,
			dstImageLayout,
			memBarriers, bufferMemBarriers, imageMemBarriers
		);

		pCmdBuffer->AttachBarriers
		(
			toBeFlushedSrcStageFlags,
			toBeFlushedDstStageFlags,
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
	VkPipelineStageFlags srcPipelineStageFlags,
	VkImageLayout srcImageLayout,
	VkAccessFlags srcAccessFlags,
	PhysicalDevice::QueueFamily	dstQueueFamily
)
{
	
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

}