#include "BufferBase.h"

bool BufferBase::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<BufferBase>& pSelf, const VkBufferCreateInfo& info)
{
	if (!VKGPUSyncRes::Init(pDevice, pSelf))
		return false;
	
	m_info = info;

	return true;
}

void BufferBase::PrepareBarriers
(
	VkAccessFlags				srcAccessFlags,
	VkImageLayout				srcImageLayout,
	VkAccessFlags				dstAccessFlags,
	VkImageLayout				dstImageLayout,
	std::vector<VkMemoryBarrier>&		memBarriers,
	std::vector<VkBufferMemoryBarrier>&	bufferMemBarriers,
	std::vector<VkImageMemoryBarrier>&	imageMemBarriers
)
{
	// We don't do specific buffer memory barrier here
	// This is recommanded by https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
	VkMemoryBarrier memBarrier = {};
	memBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memBarrier.srcAccessMask = srcAccessFlags;
	memBarrier.dstAccessMask = dstAccessFlags;

	memBarriers.push_back(memBarrier);
}

void BufferBase::PrepareQueueReleaseBarrier
(
	VkAccessFlags				srcAccessFlags,
	VkImageLayout				srcImageLayout,
	PhysicalDevice::QueueFamily	srcQueueFamily,
	PhysicalDevice::QueueFamily	dstQueueFamily,
	std::vector<VkMemoryBarrier>&		memBarriers,
	std::vector<VkBufferMemoryBarrier>&	bufferMemBarriers,
	std::vector<VkImageMemoryBarrier>&	imageMemBarriers
)
{
	VkBufferMemoryBarrier queueReleaseBarrier;
	queueReleaseBarrier = {};
	queueReleaseBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	queueReleaseBarrier.buffer = GetDeviceHandle();

	queueReleaseBarrier.srcAccessMask = srcAccessFlags;
	queueReleaseBarrier.srcQueueFamilyIndex = GetDevice()->GetPhysicalDevice()->GetQueueFamilyIndex(srcQueueFamily);

	queueReleaseBarrier.dstAccessMask = 0;
	queueReleaseBarrier.dstQueueFamilyIndex = GetDevice()->GetPhysicalDevice()->GetQueueFamilyIndex(dstQueueFamily);

	queueReleaseBarrier.offset = 0;
	queueReleaseBarrier.size = m_info.size;

	bufferMemBarriers.push_back(queueReleaseBarrier);
}

void BufferBase::PrepareQueueAcquireBarrier
(
	VkAccessFlags				dstAccessFlags,
	VkImageLayout				dstImageLayout,
	PhysicalDevice::QueueFamily	srcQueueFamily,
	PhysicalDevice::QueueFamily	dstQueueFamily,
	std::vector<VkMemoryBarrier>&		memBarriers,
	std::vector<VkBufferMemoryBarrier>&	bufferMemBarriers,
	std::vector<VkImageMemoryBarrier>&	imageMemBarriers
)
{
	VkBufferMemoryBarrier queueAcquireBarrier;
	queueAcquireBarrier = {};
	queueAcquireBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	queueAcquireBarrier.buffer = GetDeviceHandle();

	queueAcquireBarrier.srcAccessMask = 0;
	queueAcquireBarrier.srcQueueFamilyIndex = GetDevice()->GetPhysicalDevice()->GetQueueFamilyIndex(srcQueueFamily);

	queueAcquireBarrier.dstAccessMask = dstAccessFlags;
	queueAcquireBarrier.dstQueueFamilyIndex = GetDevice()->GetPhysicalDevice()->GetQueueFamilyIndex(dstQueueFamily);

	queueAcquireBarrier.offset = 0;
	queueAcquireBarrier.size = m_info.size;

	bufferMemBarriers.push_back(queueAcquireBarrier);
}