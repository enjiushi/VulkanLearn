#include "PerFrameData.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/CommandPool.h"
#include "../vulkan/DescriptorPool.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/Fence.h"

#define UINT64_MAX       0xffffffffffffffffui64

bool PerFrameData::Init(const std::shared_ptr<Device>& pDevice)
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

	//m_pFence should be set to a global frame protect fence
	
	return true;
}

std::shared_ptr<PerFrameData> PerFrameData::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<PerFrameData> pPerFrameData = std::make_shared<PerFrameData>();
	if (pPerFrameData.get() && pPerFrameData->Init(pDevice))
		return pPerFrameData;
	return nullptr;
}

void PerFrameData::WaitAndResetFence()
{
	VkFence fence = m_pFence->GetDeviceHandle();
	CHECK_VK_ERROR(vkWaitForFences(m_pFence->GetDevice()->GetDeviceHandle(), 1, &fence, VK_TRUE, UINT64_MAX));
	CHECK_VK_ERROR(vkResetFences(m_pFence->GetDevice()->GetDeviceHandle(), 1, &fence));
}