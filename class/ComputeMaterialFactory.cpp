#include "ComputeMaterialFactory.h"
#include "Material.h"
#include "CustomizedComputeMaterial.h"
#include "../common/Util.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChainImage.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"

std::shared_ptr<Material> CreateCombineMaterial()
{
	// FIXME: hard-code
	static uint32_t groupSize = 16;
	Vector3ui groupNum =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x / 16,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y / 16,
		1
	};

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