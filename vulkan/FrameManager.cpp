#include "FrameManager.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Fence.h"
#include "GlobalDeviceObjects.h"
#include "SwapChain.h"
#include "PerFrameResource.h"
#include "Fence.h"

#define UINT64_MAX       0xffffffffffffffffui64

bool FrameManager::Init(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount)
{
	for (uint32_t i = 0; i < maxFrameCount; i++)
	{
		m_frameResTable[i] = std::vector<std::shared_ptr<PerFrameResource>>();
		m_frameFences.push_back(Fence::Create(pDevice));
	}

	m_currentFrameIndex = 0;
	m_maxFrameCount = maxFrameCount;
	
	return true;
}

std::shared_ptr<FrameManager> FrameManager::Create(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount)
{
	std::shared_ptr<FrameManager> pFrameManager = std::make_shared<FrameManager>();
	if (pFrameManager.get() && pFrameManager->Init(pDevice, maxFrameCount))
		return pFrameManager;
	return nullptr;
}

std::shared_ptr<PerFrameResource> FrameManager::AllocatePerFrameResource(uint32_t frameIndex)
{
	if (frameIndex < 0 || frameIndex >= m_maxFrameCount)
		return nullptr;

	WaitForFence();

	std::shared_ptr<PerFrameResource> pPerFrameRes = PerFrameResource::Create(GlobalObjects()->GetDevice());
	m_frameResTable[frameIndex].push_back(pPerFrameRes);
	return pPerFrameRes;
}

void FrameManager::WaitForFence()
{
	if (m_frameFences[m_currentFrameIndex]->Signaled())
		return;

	VkFence fence = m_frameFences[m_currentFrameIndex]->GetDeviceHandle();
	CHECK_VK_ERROR(vkWaitForFences(GlobalObjects()->GetDevice()->GetDeviceHandle(), 1, &fence, VK_TRUE, UINT64_MAX));
	m_frameFences[m_currentFrameIndex]->Reset();
}

void FrameManager::SetFrameIndex(uint32_t index)
{
	m_currentFrameIndex = index % m_maxFrameCount;
}