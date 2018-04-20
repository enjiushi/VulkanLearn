#include "PipelineLayout.h"
#include "DescriptorSetLayout.h"

PipelineLayout::~PipelineLayout()
{
	vkDestroyPipelineLayout(GetDevice()->GetDeviceHandle(), m_pipelineLayout, nullptr);
}

bool PipelineLayout::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<PipelineLayout>& pSelf,
	const DescriptorSetLayoutList& descriptorSetLayoutList,
	const std::vector<VkPushConstantRange>& pushConstsRanges)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_descriptorSetLayoutList = descriptorSetLayoutList;

	std::vector<VkDescriptorSetLayout> dsLayoutDeviceHandleList = GetDescriptorSetLayoutDeviceHandleList();

	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.setLayoutCount = dsLayoutDeviceHandleList.size();
	info.pSetLayouts = dsLayoutDeviceHandleList.data();
	info.pushConstantRangeCount = pushConstsRanges.size();
	info.pPushConstantRanges = pushConstsRanges.data();
	CHECK_VK_ERROR(vkCreatePipelineLayout(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_pipelineLayout));

	return true;
}

std::shared_ptr<PipelineLayout> PipelineLayout::Create(const std::shared_ptr<Device>& pDevice,
	const DescriptorSetLayoutList& descriptorSetLayoutList)
{
	return Create(pDevice, descriptorSetLayoutList, {});
}

std::shared_ptr<PipelineLayout> PipelineLayout::Create(const std::shared_ptr<Device>& pDevice,
	const DescriptorSetLayoutList& descriptorSetLayoutList,
	const std::vector<VkPushConstantRange>& pushConstsRanges)
{
	std::shared_ptr<PipelineLayout> pPipelineLayout = std::make_shared<PipelineLayout>();
	if (pPipelineLayout.get() && pPipelineLayout->Init(pDevice, pPipelineLayout, descriptorSetLayoutList, pushConstsRanges))
		return pPipelineLayout;
	return nullptr;
}

const std::vector<VkDescriptorSetLayout> PipelineLayout::GetDescriptorSetLayoutDeviceHandleList()
{
	std::vector<VkDescriptorSetLayout> dsLayoutDeviceHandleList;
	dsLayoutDeviceHandleList.resize(m_descriptorSetLayoutList.size());
	for (uint32_t i = 0; i < m_descriptorSetLayoutList.size(); i++)
		dsLayoutDeviceHandleList[i] = m_descriptorSetLayoutList[i]->GetDeviceHandle();
	return dsLayoutDeviceHandleList;
}