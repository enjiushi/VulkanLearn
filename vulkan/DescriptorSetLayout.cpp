#include "DescriptorSetLayout.h"

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(GetDevice()->GetDeviceHandle(), m_descriptorSetLayout, nullptr);
}

bool DescriptorSetLayout::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<DescriptorSetLayout>& pSelf,
	const std::vector<VkDescriptorSetLayoutBinding>& dsLayoutBinding)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_descriptorSetLayoutBinding = dsLayoutBinding;

	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = m_descriptorSetLayoutBinding.size();
	createInfo.pBindings = m_descriptorSetLayoutBinding.data();
	CHECK_VK_ERROR(vkCreateDescriptorSetLayout(GetDevice()->GetDeviceHandle(), &createInfo, nullptr, &m_descriptorSetLayout));

	return true;
}

std::shared_ptr<DescriptorSetLayout> DescriptorSetLayout::Create(const std::shared_ptr<Device>& pDevice,
	const std::vector<VkDescriptorSetLayoutBinding>& dsLayoutBinding)
{
	std::shared_ptr<DescriptorSetLayout> pDsLayout = std::make_shared<DescriptorSetLayout>();
	if (pDsLayout.get() && pDsLayout->Init(pDevice, pDsLayout, dsLayoutBinding))
		return pDsLayout;
	return nullptr;
}