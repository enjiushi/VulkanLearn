#include "DescriptorSet.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"

DescriptorSet::~DescriptorSet()
{
	//Descriptor set will be destroyed when the pool allocates it is destroyed
}

bool DescriptorSet::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<DescriptorSet>& pSelf,
	const std::shared_ptr<DescriptorPool>& pDescriptorPool,
	const std::shared_ptr<DescriptorSetLayout>& pDescriptorSetLayout)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
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
	if (pDescriptorSet.get() && pDescriptorSet->Init(pDevice, pDescriptorSet, pDescriptorPool, pDescriptorSetLayout))
		return pDescriptorSet;
	return nullptr;
}

void DescriptorSet::UpdateBufferDynamic(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();
	writeData[0].pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);
}

void DescriptorSet::UpdateBuffer(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();
	writeData[0].pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);
}

void DescriptorSet::UpdateImage(uint32_t binding, const VkDescriptorImageInfo& imageInfo)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();
	writeData[0].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);
}

void DescriptorSet::UpdateTexBuffer(uint32_t binding, const VkBufferView& texBufferView)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();
	writeData[0].pTexelBufferView = &texBufferView;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);
}