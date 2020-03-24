#include "ComputeMaterialFactory.h"
#include "Material.h"
#include "CustomizedComputeMaterial.h"
#include "../common/Util.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChainImage.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"

// FIXME: hard-code
static uint32_t groupSize = 16;

std::shared_ptr<Material> CreateCombineMaterial()
{
	std::vector<std::shared_ptr<Image>> DOFResults;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pDOFResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, FrameBufferDiction::CombineLayer)[j];
		DOFResults.push_back(pDOFResult->GetColorTarget(0));
	}

	std::vector<std::shared_ptr<Image>> bloomTextures;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pBloomFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom)[j];
		bloomTextures.push_back(pBloomFrameBuffer->GetColorTarget(0));
	}

	std::vector<std::shared_ptr<Image>> combineTextures;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pCombineResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_CombineResult)[j];
		combineTextures.push_back(pCombineResult->GetColorTarget(0));
	}

	Vector3ui groupNum =
	{
		combineTextures[0]->GetImageInfo().extent.width / groupSize,
		combineTextures[0]->GetImageInfo().extent.height / groupSize,
		1
	};

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	textureUnits.push_back
	(
		{
			0,

			DOFResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				// Only a pre-barrier is needed
				{
					true,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_ACCESS_SHADER_READ_BIT
				},
				{
					false
				}
			}
		}
	);

	textureUnits.push_back
	(
		{
			1,

			bloomTextures,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				// Only a pre-barrier is needed
				{
					true,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_IMAGE_LAYOUT_GENERAL,
					VK_ACCESS_SHADER_WRITE_BIT,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_IMAGE_LAYOUT_GENERAL,
					VK_ACCESS_SHADER_READ_BIT
				},
				{
					false
				}
			}
		}
	);

	textureUnits.push_back
	(
		{
			2,

			combineTextures,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					false,
				},
				{
					false
				}
			}
		}
	);

	std::vector<uint8_t> pushConstantData;
	uint32_t dirtTextureIndex = -1;
	TransferBytesToVector(pushConstantData, &dirtTextureIndex, sizeof(dirtTextureIndex));

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/combine.comp.spv",
		groupNum,
		textureUnits,
		pushConstantData
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}

std::shared_ptr<Material> CreateBloomMaterial(BloomPass bloomPass, uint32_t iterIndex)
{
	std::wstring shaderPath;
	switch (bloomPass)
	{
	case BloomPass::PREFILTER:	shaderPath = L"../data/shaders/bloom_prefilter.comp.spv"; break;
	case BloomPass::DOWNSAMPLE: shaderPath = L"../data/shaders/bloom_downsamplebox13.comp.spv"; break;
	case BloomPass::UPSAMPLE:	shaderPath = L"../data/shaders/bloom_upsampletent.comp.spv"; break;
	default:
		ASSERTION(false);
		break;
	}

	std::vector<std::shared_ptr<Image>> inputImgs;
	std::vector<std::shared_ptr<Image>> outputImgs;

	switch (bloomPass)
	{
	case BloomPass::PREFILTER:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pDOFResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, FrameBufferDiction::CombineLayer)[j];
			inputImgs.push_back(pDOFResult->GetColorTarget(0));

			std::shared_ptr<FrameBuffer> pTarget = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, iterIndex + 1)[j];
			outputImgs.push_back(pTarget->GetColorTarget(0));
		}
		break;
	case BloomPass::DOWNSAMPLE:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPrevBloomResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, iterIndex)[j];
			inputImgs.push_back(pPrevBloomResult->GetColorTarget(0));

			std::shared_ptr<FrameBuffer> pTarget = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, iterIndex + 1)[j];
			outputImgs.push_back(pTarget->GetColorTarget(0));
		}
		break;
	case BloomPass::UPSAMPLE:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPrevBloomResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, iterIndex + 1)[j];
			inputImgs.push_back(pPrevBloomResult->GetColorTarget(0));

			std::shared_ptr<FrameBuffer> pTarget = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, iterIndex)[j];
			outputImgs.push_back(pTarget->GetColorTarget(0));
		}
		break;
	default:
		ASSERTION(false);
		break;
	}

	Vector3ui groupNum =
	{
		outputImgs[0]->GetImageInfo().extent.width / groupSize,
		outputImgs[0]->GetImageInfo().extent.height / groupSize,
		1
	};

	Vector2f size = { (float)outputImgs[0]->GetImageInfo().extent.width, (float)outputImgs[0]->GetImageInfo().extent.height };
	size *= bloomPass == BloomPass::UPSAMPLE ? 0.5f : 2.0f;
	size = { 1.0f / size.x, 1.0f / size.y };

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;

	VkPipelineStageFlagBits srcStageFlags;
	VkImageLayout srcLayout;
	VkAccessFlagBits srcAccessFlags;

	VkPipelineStageFlagBits dstStageFlags;
	VkImageLayout dstLayout;
	VkAccessFlagBits dstAccessFlags;

	switch (bloomPass)
	{
	case BloomPass::PREFILTER:
		srcStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		srcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		srcAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		dstStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dstLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		dstAccessFlags = VK_ACCESS_SHADER_READ_BIT;
		break;
	case BloomPass::DOWNSAMPLE:
		srcStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		srcLayout = VK_IMAGE_LAYOUT_GENERAL;
		srcAccessFlags = VK_ACCESS_SHADER_WRITE_BIT;

		dstStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dstLayout = VK_IMAGE_LAYOUT_GENERAL;
		dstAccessFlags = VK_ACCESS_SHADER_READ_BIT;
		break;
	case BloomPass::UPSAMPLE:
		srcStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		srcLayout = VK_IMAGE_LAYOUT_GENERAL;
		srcAccessFlags = VK_ACCESS_SHADER_WRITE_BIT;

		dstStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dstLayout = VK_IMAGE_LAYOUT_GENERAL;
		dstAccessFlags = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		ASSERTION(false);
		break;
	}

	textureUnits.push_back
	(
		{
			0,

			inputImgs,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				// Only a pre-barrier is needed
				{
					true,
					srcStageFlags,
					srcLayout,
					srcAccessFlags,

					dstStageFlags,
					dstLayout,
					dstAccessFlags
				},
				{
					false
				}
			}
		}
	);

	textureUnits.push_back
	(
		{
			1,

			outputImgs,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					false,
				},
				{
					false
				}
			}
		}
	);

	CustomizedComputeMaterial::Variables variables =
	{
		shaderPath,
		groupNum,
		textureUnits,
		{}
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}