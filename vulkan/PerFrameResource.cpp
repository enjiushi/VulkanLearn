#include "PerFrameResource.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Fence.h"
#include "GlobalDeviceObjects.h"
#include "SwapChain.h"

bool PerFrameResource::Init(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex, const std::shared_ptr<PerFrameResource>& pSelf)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_pPersistantCBPool = CommandPool::Create(pDevice, pDevice->GetPhysicalDevice()->GetGraphicQueueIndex(), pSelf);
	m_pTransientCBPool = CommandPool::CreateTransientCBPool(pDevice, pDevice->GetPhysicalDevice()->GetGraphicQueueIndex(), pSelf);

	std::vector<VkDescriptorPoolSize> descPoolSize =
	{
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2
		},
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2
		}
	};

	VkDescriptorPoolCreateInfo descPoolInfo = {};
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pPoolSizes = descPoolSize.data();
	descPoolInfo.poolSizeCount = (uint32_t)descPoolSize.size();
	descPoolInfo.maxSets = 10;
	m_pDescriptorPool = DescriptorPool::Create(pDevice, descPoolInfo);
	
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

std::shared_ptr<CommandBuffer> PerFrameResource::AllocatePersistantPrimaryCommandBuffer()
{
	return m_pPersistantCBPool->AllocatePrimaryCommandBuffer();
}

std::shared_ptr<CommandBuffer> PerFrameResource::AllocatePersistantSecondaryCommandBuffer()
{
	return m_pPersistantCBPool->AllocateSecondaryCommandBuffer();
}

std::shared_ptr<CommandBuffer> PerFrameResource::AllocateTransientPrimaryCommandBuffer()
{
	return m_pTransientCBPool->AllocatePrimaryCommandBuffer();
}

std::shared_ptr<CommandBuffer> PerFrameResource::AllocateTransientSecondaryCommandBuffer()
{
	return m_pTransientCBPool->AllocateSecondaryCommandBuffer();
}


std::shared_ptr<DescriptorSet> PerFrameResource::AllocateDescriptorSet(const std::shared_ptr<DescriptorSetLayout>& pDsLayout)
{
	return m_pDescriptorPool->AllocateDescriptorSet(pDsLayout);
}