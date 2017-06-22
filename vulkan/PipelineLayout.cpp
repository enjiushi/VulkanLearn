#include "PipelineLayout.h"
#include "DescriptorSetLayout.h"

bool PipelineLayout::Init(const std::shared_ptr<Device>& pDevice,
	const DescriptorSetLayoutList& descriptorSetLayoutList)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	m_descriptorSetLayoutList = descriptorSetLayoutList;

	std::vector<VkDescriptorSetLayout> dsLayoutDeviceHandleList;
	dsLayoutDeviceHandleList.resize(m_descriptorSetLayoutList.size());
	for (uint32_t i = 0; i < m_descriptorSetLayoutList.size(); i++)
		dsLayoutDeviceHandleList[i] = m_descriptorSetLayoutList[i]->GetDeviceHandle();

	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.setLayoutCount = dsLayoutDeviceHandleList.size();
	info.pSetLayouts = dsLayoutDeviceHandleList.data();
	CHECK_VK_ERROR(vkCreatePipelineLayout(GetDevice()->GetDeviceHandle(), &info, nullptr, &m_pipelineLayout));

	return true;
}

std::shared_ptr<PipelineLayout> PipelineLayout::Create(const std::shared_ptr<Device>& pDevice,
	const DescriptorSetLayoutList& descriptorSetLayoutList)
{
	std::shared_ptr<PipelineLayout> pPipelineLayout = std::make_shared<PipelineLayout>();
	if (pPipelineLayout.get() && pPipelineLayout->Init(pDevice, descriptorSetLayoutList))
		return pPipelineLayout;
	return nullptr;
}