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