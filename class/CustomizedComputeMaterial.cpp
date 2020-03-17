#include "CustomizedComputeMaterial.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Image.h"
#include "../vulkan/Sampler.h"
#include "../vulkan/ImageView.h"
#include "../vulkan/CommandBuffer.h"

std::shared_ptr<CustomizedComputeMaterial> CustomizedComputeMaterial::CreateMaterial(const CustomizedComputeMaterial::Variables& variables)
{
	std::shared_ptr<CustomizedComputeMaterial> pMaterial = std::make_shared<CustomizedComputeMaterial>();
	if (pMaterial.get() && pMaterial->Init(pMaterial, variables))
		return pMaterial;
	return nullptr;
}

bool CustomizedComputeMaterial::Init(const std::shared_ptr<CustomizedComputeMaterial>& pSelf, const CustomizedComputeMaterial::Variables& variables)
{
	VkComputePipelineCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

	m_variables = variables;

	VkPushConstantRange pushConstant =
	{
		VK_SHADER_STAGE_COMPUTE_BIT,
		0,
		(uint32_t)variables.pushConstantData.size()
	};

	if (!Material::Init(pSelf, variables.shaderPath, createInfo, { pushConstant }, {}, variables.groupSize))
		return false;

	for (uint32_t i = 0; i < (uint32_t)variables.textureUnits.size(); i++)
	{
		std::vector<CombinedImage> combinedImages;
		for (uint32_t j = 0; j < (uint32_t)variables.textureUnits[i].textures.size(); j++)
		{
			combinedImages.push_back
			({
				variables.textureUnits[i].textures[j],
				variables.textureUnits[i].textures[j]->CreateLinearClampToEdgeSampler(),
				variables.textureUnits[i].textures[j]->CreateDefaultImageView()
			});
		}

		m_pUniformStorageDescriptorSet->UpdateImages(variables.textureUnits[i].bindingIndex, combinedImages, true);
	}

	m_variables = variables;

	return true;
}

void CustomizedComputeMaterial::CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout)
{
	for (uint32_t i = 0; i < (uint32_t)m_variables.textureUnits.size(); i++)
	{
		materialLayout.push_back(
		{
			StorageImage,
			"Input Texture",
			{},
			(uint32_t)m_variables.textureUnits[i].textures.size()
		});
	}
}

void CustomizedComputeMaterial::CustomizePoolSize(std::vector<uint32_t>& counts)
{
	for (uint32_t i = 0; i < (uint32_t)m_variables.textureUnits.size(); i++)
	{
		counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (uint32_t)m_variables.textureUnits[i].textures.size();
	}
}

void CustomizedComputeMaterial::CustomizeCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong)
{
	pCmdBuf->PushConstants(m_pPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, (uint32_t)m_variables.pushConstantData.size(), m_variables.pushConstantData.data());
}