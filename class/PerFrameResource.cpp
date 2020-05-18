#include "PerFrameResource.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/CommandPool.h"
#include "../vulkan/DescriptorPool.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/Fence.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/SwapChain.h"

bool PerFrameResource::Init(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex, const std::shared_ptr<PerFrameResource>& pSelf)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	for (uint32_t i = 0; i < (uint32_t)PhysicalDevice::QueueFamily::COUNT; i++)
	{
		for (uint32_t j = 0; j < (uint32_t)CommandPool::CBPersistancy::COUNT; j++)
		{
			PhysicalDevice::QueueFamily queueFamily = (PhysicalDevice::QueueFamily)i;
			CommandPool::CBPersistancy persistancy = (CommandPool::CBPersistancy)j;

			m_commandPools[i][j] = CommandPool::Create(pDevice, queueFamily, persistancy, pSelf);
		}
	}

	m_frameIndex = frameIndex;

	return true;
}

std::shared_ptr<PerFrameResource> PerFrameResource::Create(const std::shared_ptr<Device>& pDevice, uint32_t frameBinIndex)
{
	std::shared_ptr<PerFrameResource> pPerFrameRes = std::make_shared<PerFrameResource>();
	if (pPerFrameRes.get() && pPerFrameRes->Init(pDevice, frameBinIndex, pPerFrameRes))
		return pPerFrameRes;
	return nullptr;
}

std::shared_ptr<CommandBuffer> PerFrameResource::AllocateCommandBuffer
(
	PhysicalDevice::QueueFamily queueFamily,
	CommandPool::CBPersistancy persistancy, 
	CommandBuffer::CBLevel level
)
{
	return m_commandPools[(uint32_t)queueFamily][(uint32_t)persistancy]->AllocateCommandBuffer(level);
}