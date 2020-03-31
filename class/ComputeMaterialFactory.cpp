#include "ComputeMaterialFactory.h"
#include "Material.h"
#include "CustomizedComputeMaterial.h"
#include "../common/Util.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChainImage.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/CommandBuffer.h"

// FIXME: hard-code
static uint32_t groupSize = 16;

std::shared_ptr<Material> CreateTemporalResolveMaterial(uint32_t pingpong)
{
	std::vector<std::shared_ptr<Image>> motionVectors;
	std::vector<std::shared_ptr<Image>> shadingResults;
	std::vector<std::shared_ptr<Image>> SSRResults;
	std::vector<std::shared_ptr<Image>> GBuffer1;
	std::vector<std::shared_ptr<Image>> motionNeighborMax;
	std::vector<std::shared_ptr<Image>> temporalShadingResults;
	std::vector<std::shared_ptr<Image>> temporalSSRResults;
	std::vector<std::shared_ptr<Image>> temporalResults;
	std::vector<std::shared_ptr<Image>> temporalCoC;
	std::vector<std::shared_ptr<Image>> outTemporalShadingResults;
	std::vector<std::shared_ptr<Image>> outTemporalSSRResults;
	std::vector<std::shared_ptr<Image>> outTemporalResults;
	std::vector<std::shared_ptr<Image>> outTemporalCoC;

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pGBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];
		motionVectors.push_back(pGBuffer->GetColorTarget(FrameBufferDiction::MotionVector));
	}

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResultBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];
		shadingResults.push_back(pShadingResultBuffer->GetColorTarget(0));
	}

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResultBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];
		SSRResults.push_back(pShadingResultBuffer->GetColorTarget(1));
	}

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pGBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];
		GBuffer1.push_back(pGBuffer->GetColorTarget(FrameBufferDiction::GBuffer1));
	}

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pMotionNeighborMax = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_MotionNeighborMax)[j];
		motionNeighborMax.push_back(pMotionNeighborMax->GetColorTarget(0));
	}

	std::shared_ptr<FrameBuffer> pTemporalResolveFB = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong);
	temporalShadingResults.push_back(pTemporalResolveFB->GetColorTarget(FrameBufferDiction::ShadingResult));
	temporalSSRResults.push_back(pTemporalResolveFB->GetColorTarget(FrameBufferDiction::SSRResult));
	temporalResults.push_back(pTemporalResolveFB->GetColorTarget(FrameBufferDiction::CombinedResult));
	temporalCoC.push_back(pTemporalResolveFB->GetColorTarget(FrameBufferDiction::CoC));

	std::shared_ptr<FrameBuffer> pOutTemporalResolveFB = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, (pingpong + 1) % 2);
	outTemporalShadingResults.push_back(pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::ShadingResult));
	outTemporalSSRResults.push_back(pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::SSRResult));
	outTemporalResults.push_back(pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::CombinedResult));
	outTemporalCoC.push_back(pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::CoC));

	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y,
	};

	Vector3ui groupNum =
	{
		size.x / groupSize,
		size.y / groupSize,
		1
	};

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	textureUnits.push_back
	(
		{
			0,

			motionVectors,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
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

			shadingResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
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

			SSRResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
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
			3,

			GBuffer1,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
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
			4,

			motionNeighborMax,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
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
			5,

			temporalShadingResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::ALL,

			{
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
			6,

			temporalSSRResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::ALL,

			{
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
			7,

			temporalResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::ALL,

			{
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
			8,

			temporalCoC,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::ALL,

			{
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
			9,

			outTemporalShadingResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::NONE,

			{
				{
					false
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
			10,

			outTemporalSSRResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::NONE,

			{
				{
					false
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
			11,

			outTemporalResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::NONE,

			{
				{
					false
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
			12,

			outTemporalCoC,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::NONE,

			{
				{
					false
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
		L"../data/shaders/temporal_resolve.comp.spv",
		groupNum,
		textureUnits,
		pushConstantData,
		[](const std::shared_ptr<CommandBuffer>& pCmdBuf, uint32_t pingpong)
		{
			std::shared_ptr<Image> pTemporalResult = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, (pingpong + 1) % 2)->GetColorTarget(FrameBufferDiction::CombinedResult);
			std::shared_ptr<Image> pTextureArray = UniformData::GetInstance()->GetGlobalTextures()->GetScreenSizeTextureArray();

			uint32_t index;
			bool ret = UniformData::GetInstance()->GetGlobalTextures()->GetScreenSizeTextureIndex("MipmapTemporalResult", index);
			ASSERTION(ret && pTextureArray != nullptr);

			Vector2d windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();
			VkImageBlit blit =
			{
				{
					VK_IMAGE_ASPECT_COLOR_BIT,
					0,
					index,
					1
				},
				{ { 0, 0, 0 },{ (int32_t)windowSize.x, (int32_t)windowSize.y, 1 } },

				{
					VK_IMAGE_ASPECT_COLOR_BIT,
					0,
					index,
					1
				},
				{ { 0, 0, 0 },{ (int32_t)windowSize.x, (int32_t)windowSize.y, 1 } }
			};
			pCmdBuf->BlitImage(pTemporalResult, pTextureArray, blit);
			pCmdBuf->GenerateMipmaps(pTextureArray, index);
		}
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}

std::shared_ptr<Material> CreateDOFMaterial(DOFPass dofPass)
{
	std::wstring shaderPath;
	switch (dofPass)
	{
	case DOFPass::PREFILTER:	shaderPath = L"../data/shaders/dof_prefilter.comp.spv"; break;
	case DOFPass::BLUR:			shaderPath = L"../data/shaders/dof_blur.comp.spv"; break;
	case DOFPass::POSTFILTER:	shaderPath = L"../data/shaders/dof_postfilter.comp.spv"; break;
	case DOFPass::COMBINE:		shaderPath = L"../data/shaders/dof_combine.comp.spv"; break;
	default:
		ASSERTION(false);
		break;
	}

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	Vector3ui groupNum;
	switch (dofPass)
	{
	case DOFPass::PREFILTER:
	{
		std::vector<std::shared_ptr<Image>> combinedTemporalResult;
		std::vector<std::shared_ptr<Image>> temporalCoCResults;
		for (uint32_t j = 0; j < 2; j++)
		{
			std::shared_ptr<FrameBuffer> pTemporalFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_TemporalResolve)[j];
			combinedTemporalResult.push_back(pTemporalFrameBuffer->GetColorTarget(FrameBufferDiction::CombinedResult));
			temporalCoCResults.push_back(pTemporalFrameBuffer->GetColorTarget(FrameBufferDiction::CoC));
		}

		std::vector<std::shared_ptr<Image>> outputImages;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPostfilterResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::PREFILTER)[j];
			outputImages.push_back(pPostfilterResult->GetColorTarget(0));
		}

		groupNum =
		{
			(uint32_t)std::ceil((double)outputImages[0]->GetImageInfo().extent.width / (double)groupSize),
			(uint32_t)std::ceil((double)outputImages[0]->GetImageInfo().extent.height / (double)groupSize),
			1
		};

		textureUnits.push_back
		(
			{
				0,

				combinedTemporalResult,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				false,

				CustomizedComputeMaterial::TextureUnit::BY_NEXTPINGPONG,

				{
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
				1,

				temporalCoCResults,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				false,

				CustomizedComputeMaterial::TextureUnit::BY_NEXTPINGPONG,

				{
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

				outputImages,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				true,

				CustomizedComputeMaterial::TextureUnit::NONE,

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
	}
	break;

	case DOFPass::BLUR:
	{
		std::vector<std::shared_ptr<Image>> prefilterResults;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPrefilterResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::PREFILTER)[j];

			prefilterResults.push_back(pPrefilterResult->GetColorTarget(0));
		}

		std::vector<std::shared_ptr<Image>> outputImages;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pBlurResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::BLUR)[j];
			outputImages.push_back(pBlurResult->GetColorTarget(0));
		}

		groupNum =
		{
			(uint32_t)std::ceil((double)outputImages[0]->GetImageInfo().extent.width / (double)groupSize),
			(uint32_t)std::ceil((double)outputImages[0]->GetImageInfo().extent.height / (double)groupSize),
			1
		};

		textureUnits.push_back
		(
			{
				0,

				prefilterResults,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				false,

				CustomizedComputeMaterial::TextureUnit::BY_FRAME,

				{
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
				1,

				outputImages,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				true,

				CustomizedComputeMaterial::TextureUnit::NONE,

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
	}
	break;
	case DOFPass::POSTFILTER:
	{
		std::vector<std::shared_ptr<Image>> blurResults;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pBlurResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::BLUR)[j];

			blurResults.push_back(pBlurResult->GetColorTarget(0));
		}

		std::vector<std::shared_ptr<Image>> outputImages;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPostfilterResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::POSTFILTER)[j];
			outputImages.push_back(pPostfilterResult->GetColorTarget(0));
		}

		groupNum =
		{
			(uint32_t)std::ceil((double)outputImages[0]->GetImageInfo().extent.width / (double)groupSize),
			(uint32_t)std::ceil((double)outputImages[0]->GetImageInfo().extent.height / (double)groupSize),
			1
		};

		textureUnits.push_back
		(
			{
				0,

				blurResults,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				false,

				CustomizedComputeMaterial::TextureUnit::BY_FRAME,

				{
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
				1,

				outputImages,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				true,

				CustomizedComputeMaterial::TextureUnit::NONE,

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
	}
	break;
	case DOFPass::COMBINE:
	{
		std::vector<std::shared_ptr<Image>> postfilterResults;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPostfilterResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::POSTFILTER)[j];
			postfilterResults.push_back(pPostfilterResult->GetColorTarget(0));
		}

		std::vector<std::shared_ptr<Image>> combinedTemporalResult;
		std::vector<std::shared_ptr<Image>> temporalCoCResults;
		for (uint32_t j = 0; j < 2; j++)
		{
			std::shared_ptr<FrameBuffer> pTemporalResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_TemporalResolve)[j];
			combinedTemporalResult.push_back(pTemporalResult->GetColorTarget(FrameBufferDiction::CombinedResult));
			temporalCoCResults.push_back(pTemporalResult->GetColorTarget(FrameBufferDiction::CoC));
		}

		std::vector<std::shared_ptr<Image>> outputImages;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pDOFCombinedResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::COMBINE)[j];
			outputImages.push_back(pDOFCombinedResult->GetColorTarget(0));
		}

		groupNum =
		{
			(uint32_t)std::ceil((double)outputImages[0]->GetImageInfo().extent.width / (double)groupSize),
			(uint32_t)std::ceil((double)outputImages[0]->GetImageInfo().extent.height / (double)groupSize),
			1
		};

		textureUnits.push_back
		(
			{
				0,

				postfilterResults,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				false,

				CustomizedComputeMaterial::TextureUnit::BY_FRAME,

				{
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
				1,

				combinedTemporalResult,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				false,

				CustomizedComputeMaterial::TextureUnit::NONE,

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

		textureUnits.push_back
		(
			{
				2,

				temporalCoCResults,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				false,

				CustomizedComputeMaterial::TextureUnit::NONE,

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

		textureUnits.push_back
		(
			{
				3,

				outputImages,
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, 0, 1 },
				true,

				CustomizedComputeMaterial::TextureUnit::NONE,

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
	}
	break;
	default:
		ASSERTION(false);
		break;
	}

	CustomizedComputeMaterial::Variables variables =
	{
		shaderPath,
		groupNum,
		textureUnits,
		{}
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
		(uint32_t)std::ceil((double)outputImgs[0]->GetImageInfo().extent.width / (double)groupSize),
		(uint32_t)std::ceil((double)outputImgs[0]->GetImageInfo().extent.height / (double)groupSize),
		1
	};

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
		srcStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		srcLayout = VK_IMAGE_LAYOUT_GENERAL;
		srcAccessFlags = VK_ACCESS_SHADER_WRITE_BIT;

		dstStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dstLayout = VK_IMAGE_LAYOUT_GENERAL;
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

			CustomizedComputeMaterial::TextureUnit::NONE,

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
			1,

			bloomTextures,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
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

			CustomizedComputeMaterial::TextureUnit::NONE,

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