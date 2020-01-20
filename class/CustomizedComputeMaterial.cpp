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

	if (!Material::Init(pSelf, variables.shaderPath, createInfo, {}, {}, variables.groupSize))
		return false;

	for (uint32_t i = 0; i < (uint32_t)variables.textures.size(); i++)
	{
		std::vector<CombinedImage> combinedImages;
		combinedImages.push_back
		({
			variables.textures[i],
			variables.textures[i]->CreateLinearClampToEdgeSampler(),
			variables.textures[i]->CreateDefaultImageView()
		});

		m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount, combinedImages);
	}

	m_variables = variables;

	return true;
}

void CustomizedComputeMaterial::CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout)
{
	for (uint32_t i = 0; i < (uint32_t)m_variables.textures.size(); i++)
	{
		materialLayout.push_back(
		{
			CombinedSampler,
			"Input Texture",
			{}
		});
	}
}

void CustomizedComputeMaterial::CustomizePoolSize(std::vector<uint32_t>& counts)
{
	counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (uint32_t)m_variables.textures.size();
}

//void CustomizedComputeMaterial::CustomizeCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong)
//{
//	//pCmdBuf->PushConstants(m_pPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GaussianBlurParams), &m_params);
//}

void CustomizedComputeMaterial::AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, uint32_t pingpong)
{
	std::vector<VkImageSubresourceRange> subresourceRanges;
	std::vector<VkImageMemoryBarrier> imgBarriers;
	for (uint32_t i = 0; (uint32_t)m_variables.textures.size(); i++)
	{
		subresourceRanges[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRanges[i].baseMipLevel = m_variables.textureSubresRanges[i].x;
		subresourceRanges[i].levelCount = m_variables.textureSubresRanges[i].y;
		subresourceRanges[i].baseArrayLayer = m_variables.textureSubresRanges[i].z;
		subresourceRanges[i].layerCount = m_variables.textureSubresRanges[i].w;

		imgBarriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgBarriers[i].image = m_variables.textures[i]->GetDeviceHandle();
		imgBarriers[i].subresourceRange = subresourceRanges[i];
		imgBarriers[i].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgBarriers[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imgBarriers[i].newLayout = VK_IMAGE_LAYOUT_GENERAL;
		imgBarriers[i].dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	}
	
	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		{},
		{},
		imgBarriers
	);
}