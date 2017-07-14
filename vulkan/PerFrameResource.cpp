#include "PerFrameResource.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Fence.h"
#include "GlobalDeviceObjects.h"
#include "SwapChain.h"

#define UINT64_MAX       0xffffffffffffffffui64

bool PerFrameResource::Init(const std::shared_ptr<Device>& pDevice)
{
	m_pCommandPool = CommandPool::Create(pDevice);

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
	descPoolInfo.poolSizeCount = descPoolSize.size();
	descPoolInfo.maxSets = 10;
	m_pDescriptorPool = DescriptorPool::Create(pDevice, descPoolInfo);
	
	return true;
}

std::shared_ptr<PerFrameResource> PerFrameResource::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<PerFrameResource> pPerFrameRes = std::make_shared<PerFrameResource>();
	if (pPerFrameRes.get() && pPerFrameRes->Init(pDevice))
		return pPerFrameRes;
	return nullptr;
}