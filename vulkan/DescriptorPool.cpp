#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(GetDevice()->GetDeviceHandle(), m_descriptorPool, nullptr);
}

bool DescriptorPool::Init(const std::shared_ptr<Device>& pDevice,
	const VkDescriptorPoolCreateInfo& info)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	m_descriptorPoolSizes.resize(info.poolSizeCount);
	m_descriptorPoolSizes.assign(info.pPoolSizes, info.pPoolSizes + info.poolSizeCount);
	
	m_descriptorPoolInfo = info;
	m_descriptorPoolInfo.pPoolSizes = m_descriptorPoolSizes.data();

	CHECK_VK_ERROR(vkCreateDescriptorPool(GetDevice()->GetDeviceHandle(), &m_descriptorPoolInfo, nullptr, &m_descriptorPool));

	return true;
}

std::shared_ptr<DescriptorPool> DescriptorPool::Create(const std::shared_ptr<Device>& pDevice,
	const VkDescriptorPoolCreateInfo& info)
{
	std::shared_ptr<DescriptorPool> pDsPool = std::make_shared<DescriptorPool>();
	if (pDsPool.get() && pDsPool->Init(pDevice, info))
		return pDsPool;
	return nullptr;
}

std::shared_ptr<DescriptorSet> DescriptorPool::AllocateDescriptorSet(const std::shared_ptr<DescriptorPool>& pDescriptorPool, const std::shared_ptr<DescriptorSetLayout>& pDescriptorSetLayout)
{
	return DescriptorSet::Create(pDescriptorPool->GetDevice(), pDescriptorPool, pDescriptorSetLayout);
}