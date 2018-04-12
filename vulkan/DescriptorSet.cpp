#include "DescriptorSet.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "Buffer.h"
#include "Image.h"
#include "UniformBuffer.h"
#include "ShaderStorageBuffer.h"
#include "ImageView.h"
#include "Sampler.h"

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

void DescriptorSet::UpdateUniformBufferDynamic(uint32_t binding, const std::shared_ptr<UniformBuffer>& pBuffer)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();

	VkDescriptorBufferInfo info = pBuffer->GetDescBufferInfo();
	writeData[0].pBufferInfo = &info;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);

	m_resourceTable[binding].push_back(pBuffer);
}

void DescriptorSet::UpdateUniformBuffer(uint32_t binding, const std::shared_ptr<UniformBuffer>& pBuffer)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();

	VkDescriptorBufferInfo info = pBuffer->GetDescBufferInfo();
	writeData[0].pBufferInfo = &info;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);

	m_resourceTable[binding].push_back(pBuffer);
}

void DescriptorSet::UpdateImage(uint32_t binding, const std::shared_ptr<Image>& pImage, const std::shared_ptr<Sampler> pSampler, const std::shared_ptr<ImageView> pImageView)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();

	VkDescriptorImageInfo info = {};
	info.imageLayout = pImage->GetImageInfo().initialLayout;
	info.imageView = pImageView->GetDeviceHandle();
	info.sampler = pSampler->GetDeviceHandle();
	writeData[0].pImageInfo = &info;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);

	m_resourceTable[binding].push_back(pImage);

	AddToReferenceTable(pSampler);
	AddToReferenceTable(pImageView);
}

void DescriptorSet::UpdateImage(uint32_t binding, const CombinedImage& image)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();

	VkDescriptorImageInfo info = {};
	info.imageLayout = image.pImage->GetImageInfo().initialLayout;
	info.imageView = image.pImageView->GetDeviceHandle();
	info.sampler = image.pSampler->GetDeviceHandle();
	writeData[0].pImageInfo = &info;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);

	m_resourceTable[binding].push_back(image.pImage);

	AddToReferenceTable(image.pSampler);
	AddToReferenceTable(image.pImageView);
}

void DescriptorSet::UpdateImages(uint32_t binding, const std::vector<CombinedImage>& images)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = images.size();
	writeData[0].dstSet = GetDeviceHandle();

	std::vector<VkDescriptorImageInfo> info;
	for (uint32_t i = 0; i < images.size(); i++)
	{
		info.push_back({
			images[i].pSampler->GetDeviceHandle(),
			images[i].pImageView->GetDeviceHandle(),
			images[i].pImage->GetImageInfo().initialLayout
			});

		m_resourceTable[binding].push_back(images[i].pImage);
		AddToReferenceTable(images[i].pSampler);
		AddToReferenceTable(images[i].pImageView);
	}
	writeData[0].pImageInfo = info.data();

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);
}

void DescriptorSet::UpdateInputImage(uint32_t binding, const std::shared_ptr<Image>& pImage, const std::shared_ptr<Sampler> pSampler, const std::shared_ptr<ImageView> pImageView)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();

	VkDescriptorImageInfo info = {};
	info.imageLayout = pImage->GetImageInfo().initialLayout;
	info.imageView = pImageView->GetDeviceHandle();
	info.sampler = pSampler->GetDeviceHandle();

	writeData[0].pImageInfo = &info;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);

	m_resourceTable[binding].push_back(pImage);

	AddToReferenceTable(pSampler);
	AddToReferenceTable(pImageView);
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

void DescriptorSet::UpdateShaderStorageBufferDynamic(uint32_t binding, const std::shared_ptr<ShaderStorageBuffer>& pBuffer)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();

	VkDescriptorBufferInfo info = pBuffer->GetDescBufferInfo();
	writeData[0].pBufferInfo = &info;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);

	m_resourceTable[binding].push_back(pBuffer);
}

void DescriptorSet::UpdateShaderStorageBuffer(uint32_t binding, const std::shared_ptr<ShaderStorageBuffer>& pBuffer)
{
	std::vector<VkWriteDescriptorSet> writeData = { {} };
	writeData[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeData[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeData[0].dstBinding = binding;
	writeData[0].descriptorCount = 1;
	writeData[0].dstSet = GetDeviceHandle();

	VkDescriptorBufferInfo info = pBuffer->GetDescBufferInfo();
	writeData[0].pBufferInfo = &info;

	vkUpdateDescriptorSets(GetDevice()->GetDeviceHandle(), writeData.size(), writeData.data(), 0, nullptr);

	m_resourceTable[binding].push_back(pBuffer);
}