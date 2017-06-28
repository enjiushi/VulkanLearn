#include "DescriptorSet.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"

DescriptorSet::~DescriptorSet()
{
	//Descriptor set will be destroyed when the pool allocates it is destroyed
}

bool DescriptorSet::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<DescriptorPool>& pDescriptorPool,
	const std::shared_ptr<DescriptorSetLayout>& pDescriptorSetLayout)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = pDescriptorPool->GetDeviceHandle();

	std::vector<VkDescriptorSetLayout> dsSetLayout =
	{
		pDescriptorSetLayout->GetDeviceHandle(),
	};
	allocateInfo.descriptorSetCount = dsSetLayout.size();
	allocateInfo.pSetLayouts = dsSetLayout.data();

	CHECK_VK_ERROR(vkAllocateDescriptorSets(GetDevice()->GetDeviceHandle(), &allocateInfo, &m_descriptorSet));
	return true;
}

std::shared_ptr<DescriptorSet> DescriptorSet::Create(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<DescriptorPool>& pDescriptorPool,
	const std::shared_ptr<DescriptorSetLayout>& pDescriptorSetLayout)
{
	std::shared_ptr<DescriptorSet> pDescriptorSet = std::make_shared<DescriptorSet>();
	if (pDescriptorSet.get() && pDescriptorSet->Init(pDevice, pDescriptorPool, pDescriptorSetLayout))
		return pDescriptorSet;
	return nullptr;
}