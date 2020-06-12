#pragma once

#include "VKFenceGuardRes.h"
#include <set>

class Semaphore;
class CommandBuffer;

class VKGPUSyncRes : public VKFenceGuardRes
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<VKGPUSyncRes>& pSelf);

public:
	virtual void PrepareBarriers
	(
		VkAccessFlags				srcAccessFlags,
		VkImageLayout				srcImageLayout,
		VkAccessFlags				dstAccessFlags,
		VkImageLayout				dstImageLayout,
		std::vector<VkMemoryBarrier>&		memBarriers,
		std::vector<VkBufferMemoryBarrier>&	bufferMemBarriers,
		std::vector<VkImageMemoryBarrier>&	imageMemBarriers
	) = 0;

private:
	std::set<std::shared_ptr<Semaphore>>	m_guardSemaphores;
};