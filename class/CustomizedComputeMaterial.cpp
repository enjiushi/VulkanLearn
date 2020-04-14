#include "CustomizedComputeMaterial.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Image.h"
#include "../vulkan/Sampler.h"
#include "../vulkan/ImageView.h"
#include "../vulkan/CommandBuffer.h"
#include "../common/Util.h"

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

	std::vector<VkPushConstantRange> pushConstants;

	if (variables.pushConstantData.size() > 0)
	{
		pushConstants.push_back
		({
			VK_SHADER_STAGE_COMPUTE_BIT,
			0,
			(uint32_t)variables.pushConstantData.size()
		});
	}

	if (!Material::Init(pSelf, variables.shaderPath, createInfo, pushConstants, {}, variables.groupSize))
		return false;

	for (uint32_t i = 0; i < (uint32_t)variables.textureUnits.size(); i++)
	{
		std::vector<CombinedImage> combinedImages;
		for (uint32_t j = 0; j < (uint32_t)variables.textureUnits[i].textures.size(); j++)
		{
			combinedImages.push_back(variables.textureUnits[i].textures[j]);
		}

		m_pUniformStorageDescriptorSet->UpdateImages(variables.textureUnits[i].bindingIndex, combinedImages, variables.textureUnits[i].isStorageImage);
	}

	m_variables = variables;

	return true;
}

void CustomizedComputeMaterial::CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout)
{
	for (uint32_t i = 0; i < (uint32_t)m_variables.textureUnits.size(); i++)
	{
		MaterialVariableType type = m_variables.textureUnits[i].isStorageImage ? StorageImage : CombinedSampler;
		materialLayout.push_back(
		{
			type,
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
		VkDescriptorType descriptorType = m_variables.textureUnits[i].isStorageImage ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		counts[descriptorType] += (uint32_t)m_variables.textureUnits[i].textures.size();
	}
}

void CustomizedComputeMaterial::CustomizeCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong)
{
	if (m_variables.pushConstantData.size() == 0)
		return;
	pCmdBuf->PushConstants(m_pPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, (uint32_t)m_variables.pushConstantData.size(), m_variables.pushConstantData.data());
}

void CustomizedComputeMaterial::AfterRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, uint32_t pingpong)
{
	Material::AfterRenderPass(pCmdBuf, pingpong);
	m_variables.customFunctionAfterDispatch(pCmdBuf, pingpong);
}

void CustomizedComputeMaterial::AssembleBarrier(const TextureUnit& textureUnit, uint32_t textureIndex, BarrierInsertionPoint barrierInsertPoint, VkImageMemoryBarrier& barrier, VkImageSubresourceRange& subresRange)
{
	subresRange = {};
	subresRange.aspectMask		= textureUnit.aspectMask;
	subresRange.baseMipLevel	= textureUnit.textureSubresRange.x;
	subresRange.levelCount		= textureUnit.textureSubresRange.y;
	subresRange.baseArrayLayer	= textureUnit.textureSubresRange.z;
	subresRange.layerCount		= textureUnit.textureSubresRange.w;

	barrier = {};
	barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image				= textureUnit.textures[textureIndex].pImage->GetDeviceHandle();
	barrier.subresourceRange	= subresRange;
	barrier.oldLayout			= textureUnit.textureBarrier[barrierInsertPoint].oldImageLayout;
	barrier.srcAccessMask		= textureUnit.textureBarrier[barrierInsertPoint].srcAccessFlags;
	barrier.newLayout			= textureUnit.textureBarrier[barrierInsertPoint].newImageLayout;
	barrier.dstAccessMask		= textureUnit.textureBarrier[barrierInsertPoint].dstAccessFlags;
}

void CustomizedComputeMaterial::AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, BarrierInsertionPoint barrierInsertionPoint, uint32_t pingpong)
{
	if (barrierInsertionPoint == Material::BarrierInsertionPoint::AFTER_DISPATCH)
		return;

	std::vector<VkImageMemoryBarrier> barriers;
	VkPipelineStageFlags srcStages = 0;
	VkPipelineStageFlags dstStages = 0;
	VkImageMemoryBarrier imgBarrier;
	VkImageSubresourceRange subresourceRange;

	for (auto textureUnit : m_variables.textureUnits)
	{
		// Skip barrier construction if current texture unit don't want one
		if (textureUnit.textureBarrier[barrierInsertionPoint].enableBarrier == false)
			continue;

		if (textureUnit.textureSelector == TextureUnit::BY_FRAME)
		{ 
			AssembleBarrier(textureUnit, FrameMgr()->FrameIndex(), barrierInsertionPoint, imgBarrier, subresourceRange);
			barriers.push_back(imgBarrier);
		}
		else if (textureUnit.textureSelector == TextureUnit::BY_PINGPONG)
		{
			AssembleBarrier(textureUnit, pingpong, barrierInsertionPoint, imgBarrier, subresourceRange);
			barriers.push_back(imgBarrier);
		}
		else if (textureUnit.textureSelector == TextureUnit::BY_NEXTPINGPONG)
		{
			AssembleBarrier(textureUnit, (pingpong + 1) % 2, barrierInsertionPoint, imgBarrier, subresourceRange);
			barriers.push_back(imgBarrier);
		}
		else if (textureUnit.textureSelector == TextureUnit::ALL)
		{
			for (uint32_t i = 0; i < (uint32_t)textureUnit.textures.size(); i++)
			{
				AssembleBarrier(textureUnit, i, barrierInsertionPoint, imgBarrier, subresourceRange);
				barriers.push_back(imgBarrier);
			}
		}

		srcStages |= textureUnit.textureBarrier[barrierInsertionPoint].srcPipelineStages;
		dstStages |= textureUnit.textureBarrier[barrierInsertionPoint].dstPipelineStages;
	}

	if (barriers.size() == 0)
		return;

	pCmdBuffer->AttachBarriers
	(
		srcStages,
		dstStages,
		{},
		{},
		barriers
	);
}

void CustomizedComputeMaterial::UpdatePushConstantDataInternal(const void* pData, uint32_t size)
{
	TransferBytesToVector(m_variables.pushConstantData, pData, size);
}