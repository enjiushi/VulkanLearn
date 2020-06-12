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

std::shared_ptr<Material> CreateSkyboxGenMaterial(const std::vector<std::shared_ptr<Image>>& outputImages)
{
	std::vector<CombinedImage> _outputImages;
	for (auto pOutputImage : outputImages)
		_outputImages.push_back
		({
			pOutputImage,
			pOutputImage->CreateLinearClampToEdgeSampler(),
			pOutputImage->CreateDefaultImageView(true)
		});

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	textureUnits.push_back
	(
		{
			0,

			_outputImages,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::ALL,

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

	Vector3ui groupNum =
	{
		(uint32_t)std::ceil((double)_outputImages[0].pImage->GetImageInfo().extent.width / (double)groupSize),
		(uint32_t)std::ceil((double)_outputImages[0].pImage->GetImageInfo().extent.height / (double)groupSize),
		1
	};

	// xyz0: bottom left corner. w0: face id
	// xyz1: bottom right corner. w1: padding
	// xyz2: top left corner. w2: padding
	// xyz3: top right corner. w3: pingpoing index
	// xyz4: world space camera position
	// xyz5: world space main light direction
	std::vector<uint8_t> pushConstantData;
	pushConstantData.resize(sizeof(Vector4f) * 6);

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/env_skybox_gen.comp.spv",
		groupNum,
		textureUnits,
		pushConstantData
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}

std::shared_ptr<Material> CreateIrradianceGenMaterial(const std::vector<std::shared_ptr<Image>>& inputImages, const std::vector<std::shared_ptr<Image>>& outputImages)
{
	std::vector<CombinedImage> _inputImages;
	for (auto pInputImage : inputImages)
		_inputImages.push_back
		({
			pInputImage,
			pInputImage->CreateLinearClampToEdgeSampler(),
			pInputImage->CreateDefaultImageView()
		});

	std::vector<CombinedImage> _outputImages;
	for (auto pOutputImage : outputImages)
	_outputImages.push_back
		({
			pOutputImage,
			pOutputImage->CreateLinearClampToEdgeSampler(),
			pOutputImage->CreateDefaultImageView(true)
		});

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	textureUnits.push_back
	(
		{
			0,

			_inputImages,
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
			1,

			_outputImages,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::ALL,

			{
				{
					false
				},
				{
					true,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_IMAGE_LAYOUT_GENERAL,
					VK_ACCESS_SHADER_WRITE_BIT,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_IMAGE_LAYOUT_GENERAL,
					VK_ACCESS_SHADER_READ_BIT
				}
			}
		}
	);

	Vector3ui groupNum = { 8, 8, 1 };

	// xyz0: bottom left corner. w0: face id
	// xyz1: bottom right corner. w1: group offset x
	// xyz2: top left corner. w2: group offset y
	// xyz3: top right corner. w3: padding
	std::vector<uint8_t> pushConstantData;
	pushConstantData.resize(sizeof(Vector4f) * 4);

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/env_irradiance_gen.comp.spv",
		groupNum,
		textureUnits,
		pushConstantData
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}

std::shared_ptr<Material> CreateReflectionGenMaterial(const std::vector<std::shared_ptr<Image>>& inputImages, const std::vector<std::shared_ptr<Image>>& outputImages, uint32_t outMipLevel)
{
	std::vector<CombinedImage> _inputImages;
	for (auto pInputImage : inputImages)
		_inputImages.push_back
		({
			pInputImage,
			pInputImage->CreateLinearClampToEdgeSampler(),
			pInputImage->CreateDefaultImageView()
		});

	std::vector<CombinedImage> _outputImages;
	for (auto pOutputImage : outputImages)
		_outputImages.push_back
		({
			pOutputImage,
			pOutputImage->CreateLinearClampToEdgeSampler(),
			pOutputImage->CreateImageView(outMipLevel, true)
		});

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	textureUnits.push_back
	(
		{
			0,

			_inputImages,
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
			1,

			_outputImages,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ outMipLevel, 1, 0, outputImages[0]->GetImageInfo().arrayLayers },
			true,

			CustomizedComputeMaterial::TextureUnit::ALL,

			{
				{
					false
				},
				{
					true,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_IMAGE_LAYOUT_GENERAL,
					VK_ACCESS_SHADER_WRITE_BIT,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_IMAGE_LAYOUT_GENERAL,
					VK_ACCESS_SHADER_READ_BIT
				}
			}
		}
	);

	Vector3ui groupNum = { outputImages[0]->GetImageInfo().extent.width >> outMipLevel, outputImages[0]->GetImageInfo().extent.height >> outMipLevel, 1 };

	// xyz0: bottom left corner. w0: face id
	// xyz1: bottom right corner. w1: roughness
	// xyz2: top left corner. w2: padding
	// xyz3: top right corner. w3: padding
	std::vector<uint8_t> pushConstantData;
	pushConstantData.resize(sizeof(Vector4f) * 4);

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/env_reflection_gen.comp.spv",
		groupNum,
		textureUnits,
		pushConstantData
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}

std::shared_ptr<Material> CreateTileMaxMaterial(const std::vector<std::shared_ptr<Image>>& inputImages, const std::vector<std::shared_ptr<Image>>& outputImages)
{
	std::vector<CombinedImage> _inputImages;
	for (auto pImage : inputImages)
	{
		_inputImages.push_back
		({
			pImage,
			pImage->CreateLinearClampToEdgeSampler(),
			pImage->CreateDefaultImageView()
		});
	}

	std::vector<CombinedImage> _outputImages;
	for (auto pImage : outputImages)
	{
		_outputImages.push_back
		({
			pImage,
			pImage->CreateLinearClampToEdgeSampler(),
			pImage->CreateDefaultImageView()
		});
	}

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	textureUnits.push_back
	(
		{
			0,

			_inputImages,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					true,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					inputImages[0]->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					inputImages[0]->GetImageInfo().initialLayout,
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

			_outputImages,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					// False means a claim, new mechanism. Old one will be removed later
					false,

					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outputImages[0]->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
				},
				{
					false
				}
			}
		}
	);

	Vector3ui groupNum =
	{
		(uint32_t)std::ceil((double)_outputImages[0].pImage->GetImageInfo().extent.width / (double)groupSize),
		(uint32_t)std::ceil((double)_outputImages[0].pImage->GetImageInfo().extent.height / (double)groupSize),
		1
	};

	std::vector<uint8_t> pushConstantData;

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/tile_max.comp.spv",
		groupNum,
		textureUnits,
		pushConstantData
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}

std::shared_ptr<Material> CreateNeighborMaxMaterial(const std::vector<std::shared_ptr<Image>>& inputImages, const std::vector<std::shared_ptr<Image>>& outputImages)
{
	std::vector<CombinedImage> _inputImages;
	for (auto pImage : inputImages)
	{
		_inputImages.push_back
		({
			pImage,
			pImage->CreateLinearClampToEdgeSampler(),
			pImage->CreateDefaultImageView()
			});
	}

	std::vector<CombinedImage> _outputImages;
	for (auto pImage : outputImages)
	{
		_outputImages.push_back
		({
			pImage,
			pImage->CreateLinearClampToEdgeSampler(),
			pImage->CreateDefaultImageView()
			});
	}

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	textureUnits.push_back
	(
		{
			0,

			_inputImages,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					true,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					inputImages[0]->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					inputImages[0]->GetImageInfo().initialLayout,
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

			_outputImages,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outputImages[0]->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
				},
				{
					false
				}
			}
		}
	);

	Vector3ui groupNum =
	{
		(uint32_t)std::ceil((double)_outputImages[0].pImage->GetImageInfo().extent.width / (double)groupSize),
		(uint32_t)std::ceil((double)_outputImages[0].pImage->GetImageInfo().extent.height / (double)groupSize),
		1
	};

	std::vector<uint8_t> pushConstantData;

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/neighbor_max.comp.spv",
		groupNum,
		textureUnits,
		pushConstantData
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}

std::shared_ptr<Material> CreateSSAOSSRMaterial()
{
	std::vector<CombinedImage> gbuffer0;
	std::vector<CombinedImage> gbuffer2;
	std::vector<CombinedImage> depthBuffer;
	std::vector<CombinedImage> outSSAOFactor;
	std::vector<CombinedImage> outSSRInfo;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pGBufferFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];

		gbuffer0.push_back
		({
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer0),
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer0)->CreateLinearClampToEdgeSampler(),
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer0)->CreateDefaultImageView()
		});

		gbuffer2.push_back
		({
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer2),
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer2)->CreateLinearClampToEdgeSampler(),
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer2)->CreateDefaultImageView()
		});

		depthBuffer.push_back
		({
			pGBufferFrameBuffer->GetDepthStencilTarget(),
			pGBufferFrameBuffer->GetDepthStencilTarget()->CreateLinearClampToBorderSampler(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK),
			pGBufferFrameBuffer->GetDepthStencilTarget()->CreateDepthSampleImageView()
		});

		std::shared_ptr<FrameBuffer> pSSAOSSRFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_SSAOSSR)[j];

		outSSAOFactor.push_back
		({
			pSSAOSSRFrameBuffer->GetColorTarget(0),
			pSSAOSSRFrameBuffer->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pSSAOSSRFrameBuffer->GetColorTarget(0)->CreateDefaultImageView()
		});

		outSSRInfo.push_back
		({
			pSSAOSSRFrameBuffer->GetColorTarget(1),
			pSSAOSSRFrameBuffer->GetColorTarget(1)->CreateLinearClampToEdgeSampler(),
			pSSAOSSRFrameBuffer->GetColorTarget(1)->CreateDefaultImageView()
		});
	}

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	textureUnits.push_back
	(
		{
			0,

			gbuffer0,
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

			gbuffer2,
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

			depthBuffer,
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					true,
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,

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

			outSSAOFactor,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outSSAOFactor[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
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

			outSSRInfo,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outSSRInfo[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
				},
				{
					false
				}
			}
		}
	);

	uint32_t index;
	UniformData::GetInstance()->GetGlobalTextures()->GetTextureIndex(RGBA8_1024, "BlueNoise", index);
	float floatIndex = (float)index;

	std::vector<uint8_t> pushConstantData;
	TransferBytesToVector(pushConstantData, &floatIndex, 0, sizeof(floatIndex));

	Vector3ui groupNum =
	{
		outSSAOFactor[0].pImage->GetImageInfo().extent.width / groupSize,
		outSSAOFactor[0].pImage->GetImageInfo().extent.height / groupSize,
		1
	};

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/ssao_ssr_gen.comp.spv",
		groupNum,
		textureUnits,
		pushConstantData
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}

std::shared_ptr<Material> CreateGaussianBlurMaterial(const std::vector<std::shared_ptr<Image>>& inputImages, const std::vector<std::shared_ptr<Image>>& outputImages, const GaussianBlurParams& params)
{
	std::vector<CombinedImage> _inputImages;
	for (auto pImage : inputImages)
	{
		_inputImages.push_back
		({
			pImage,
			pImage->CreateLinearClampToEdgeSampler(),
			pImage->CreateDefaultImageView()
		});
	}

	std::vector<CombinedImage> _outputImages;
	for (auto pImage : outputImages)
	{
		_outputImages.push_back
		({
			pImage,
			pImage->CreateLinearClampToEdgeSampler(),
			pImage->CreateDefaultImageView()
		});
	}

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	textureUnits.push_back
	(
		{
			0,

			_inputImages,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					true,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					inputImages[0]->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					inputImages[0]->GetImageInfo().initialLayout,
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

			_outputImages,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					_outputImages[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
				},
				{
					false
				}
			}
		}
	);

	Vector3ui groupNum =
	{
		inputImages[0]->GetImageInfo().extent.width / groupSize,
		inputImages[0]->GetImageInfo().extent.height / groupSize,
		1
	};

	std::vector<uint8_t> pushConstantData;
	TransferBytesToVector(pushConstantData, &params, 0, sizeof(params));

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/gaussian_blur.comp.spv",
		groupNum,
		textureUnits,
		pushConstantData
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}

std::shared_ptr<Material> CreateDeferredShadingMaterial()
{
	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;

	for (uint32_t i = 0; i < FrameBufferDiction::GBufferCount; i++)
	{
		std::vector<CombinedImage> gbuffers;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pGBufferFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];
			gbuffers.push_back
			({
				pGBufferFrameBuffer->GetColorTarget(i),
				pGBufferFrameBuffer->GetColorTarget(i)->CreateLinearClampToEdgeSampler(),
				pGBufferFrameBuffer->GetColorTarget(i)->CreateDefaultImageView()
			});
		}

		textureUnits.push_back
		(
			{
				i,

				gbuffers,
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
	}

	std::vector<CombinedImage> depthStencil;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pGBufferFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];
		depthStencil.push_back
		({
			pGBufferFrameBuffer->GetDepthStencilTarget(),
			pGBufferFrameBuffer->GetDepthStencilTarget()->CreateLinearClampToEdgeSampler(),
			pGBufferFrameBuffer->GetDepthStencilTarget()->CreateDepthSampleImageView()
		});
	}

	textureUnits.push_back
	(
		{
			4,

			depthStencil,
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					true,
					VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,

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

	std::vector<CombinedImage> shadowMap;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadowPassFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_ShadowMap)[j];
		shadowMap.push_back
		({
			pShadowPassFrameBuffer->GetDepthStencilTarget(),
			pShadowPassFrameBuffer->GetDepthStencilTarget()->CreateLinearClampToBorderSampler(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK),
			pShadowPassFrameBuffer->GetDepthStencilTarget()->CreateDepthSampleImageView()
		});
	}

	textureUnits.push_back
	(
		{
			5,

			shadowMap,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					true,
					VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,

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

	std::vector<CombinedImage> blurredSSAOBuffers;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_SSAOBlurH)[j];
		blurredSSAOBuffers.push_back
		({
			pFrameBuffer->GetColorTarget(0),
			pFrameBuffer->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pFrameBuffer->GetColorTarget(0)->CreateDefaultImageView()
		});
	}

	textureUnits.push_back
	(
		{
			6,

			blurredSSAOBuffers,
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

	std::vector<CombinedImage> SSRInfoBuffers;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_SSAOSSR)[j];

		SSRInfoBuffers.push_back
		({
			pFrameBuffer->GetColorTarget(1),
			pFrameBuffer->GetColorTarget(1)->CreateLinearClampToEdgeSampler(),
			pFrameBuffer->GetColorTarget(1)->CreateDefaultImageView()
		});
	}

	textureUnits.push_back
	(
		{
			7,

			SSRInfoBuffers,
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

	std::vector<CombinedImage> outShadingResults;
	std::vector<CombinedImage> outSSRResults;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResultBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];
		outShadingResults.push_back
		({
			pShadingResultBuffer->GetColorTarget(0),
			pShadingResultBuffer->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pShadingResultBuffer->GetColorTarget(0)->CreateDefaultImageView()
		});
	}

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResultBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];
		outSSRResults.push_back
		({
			pShadingResultBuffer->GetColorTarget(1),
			pShadingResultBuffer->GetColorTarget(1)->CreateLinearClampToEdgeSampler(),
			pShadingResultBuffer->GetColorTarget(1)->CreateDefaultImageView()
		});
	}

	textureUnits.push_back
	(
		{
			8,

			outShadingResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outShadingResults[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
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

			outSSRResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			true,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outSSRResults[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
				},
				{
					false
				}
			}
		}
	);

	Vector3ui groupNum =
	{
		(uint32_t)std::ceil((double)outShadingResults[0].pImage->GetImageInfo().extent.width / (double)groupSize),
		(uint32_t)std::ceil((double)outShadingResults[0].pImage->GetImageInfo().extent.height / (double)groupSize),
		1
	};

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/pbr_deferred_shading.comp.spv",
		groupNum,
		textureUnits,
		{}
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}

std::shared_ptr<Material> CreateTemporalResolveMaterial(uint32_t pingpong)
{
	std::vector<CombinedImage> motionVectors;
	std::vector<CombinedImage> shadingResults;
	std::vector<CombinedImage> SSRResults;
	std::vector<CombinedImage> GBuffer1;
	std::vector<CombinedImage> motionNeighborMax;
	std::vector<CombinedImage> temporalShadingResults;
	std::vector<CombinedImage> temporalSSRResults;
	std::vector<CombinedImage> temporalResults;
	std::vector<CombinedImage> temporalCoC;
	std::vector<CombinedImage> outTemporalShadingResults;
	std::vector<CombinedImage> outTemporalSSRResults;
	std::vector<CombinedImage> outTemporalResults;
	std::vector<CombinedImage> outTemporalCoC;

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pGBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];
		motionVectors.push_back
		({
			pGBuffer->GetColorTarget(FrameBufferDiction::MotionVector),
			pGBuffer->GetColorTarget(FrameBufferDiction::MotionVector)->CreateLinearClampToEdgeSampler(),
			pGBuffer->GetColorTarget(FrameBufferDiction::MotionVector)->CreateDefaultImageView()
		});
	}

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResultBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];
		shadingResults.push_back
		({
			pShadingResultBuffer->GetColorTarget(0),
			pShadingResultBuffer->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pShadingResultBuffer->GetColorTarget(0)->CreateDefaultImageView()
		});
	}

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResultBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];
		SSRResults.push_back
		({
			pShadingResultBuffer->GetColorTarget(1),
			pShadingResultBuffer->GetColorTarget(1)->CreateLinearClampToEdgeSampler(),
			pShadingResultBuffer->GetColorTarget(1)->CreateDefaultImageView()
		});
	}

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pGBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];
		GBuffer1.push_back
		({
			pGBuffer->GetColorTarget(FrameBufferDiction::GBuffer1),
			pGBuffer->GetColorTarget(FrameBufferDiction::GBuffer1)->CreateLinearClampToEdgeSampler(),
			pGBuffer->GetColorTarget(FrameBufferDiction::GBuffer1)->CreateDefaultImageView()
		});
	}

	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pMotionNeighborMax = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_MotionNeighborMax)[j];
		motionNeighborMax.push_back
		({
			pMotionNeighborMax->GetColorTarget(0),
			pMotionNeighborMax->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pMotionNeighborMax->GetColorTarget(0)->CreateDefaultImageView()
		});
	}

	std::shared_ptr<FrameBuffer> pTemporalResolveFB = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong);
	temporalShadingResults.push_back
	({
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::ShadingResult),
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::ShadingResult)->CreateLinearClampToEdgeSampler(),
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::ShadingResult)->CreateDefaultImageView()
	});
	temporalSSRResults.push_back
	({
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::SSRResult),
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::SSRResult)->CreateLinearClampToEdgeSampler(),
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::SSRResult)->CreateDefaultImageView()
	});
	temporalResults.push_back
	({
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::CombinedResult),
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::CombinedResult)->CreateLinearClampToEdgeSampler(),
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::CombinedResult)->CreateDefaultImageView()
	});
	temporalCoC.push_back
	({
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::CoC),
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::CoC)->CreateLinearClampToEdgeSampler(),
		pTemporalResolveFB->GetColorTarget(FrameBufferDiction::CoC)->CreateDefaultImageView()
	});

	std::shared_ptr<FrameBuffer> pOutTemporalResolveFB = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, (pingpong + 1) % 2);
	outTemporalShadingResults.push_back
	({
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::ShadingResult),
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::ShadingResult)->CreateLinearClampToEdgeSampler(),
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::ShadingResult)->CreateDefaultImageView()
	});
	outTemporalSSRResults.push_back
	({
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::SSRResult),
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::SSRResult)->CreateLinearClampToEdgeSampler(),
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::SSRResult)->CreateDefaultImageView()
	});
	outTemporalResults.push_back
	({
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::CombinedResult),
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::CombinedResult)->CreateLinearClampToEdgeSampler(),
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::CombinedResult)->CreateDefaultImageView()
	});
	outTemporalCoC.push_back
	({
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::CoC),
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::CoC)->CreateLinearClampToEdgeSampler(),
		pOutTemporalResolveFB->GetColorTarget(FrameBufferDiction::CoC)->CreateDefaultImageView()
	});

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

			SSRResults,
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

			CustomizedComputeMaterial::TextureUnit::ALL,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outTemporalShadingResults[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
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

			CustomizedComputeMaterial::TextureUnit::ALL,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outTemporalSSRResults[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
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

			CustomizedComputeMaterial::TextureUnit::ALL,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outTemporalResults[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
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

			CustomizedComputeMaterial::TextureUnit::ALL,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outTemporalCoC[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
				},
				{
					false
				}
			}
		}
	);

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/temporal_resolve.comp.spv",
		groupNum,
		textureUnits,
		{},
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
		std::vector<CombinedImage> combinedTemporalResult;
		std::vector<CombinedImage> temporalCoCResults;
		for (uint32_t j = 0; j < 2; j++)
		{
			std::shared_ptr<FrameBuffer> pTemporalFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_TemporalResolve)[j];
			combinedTemporalResult.push_back
			({
				pTemporalFrameBuffer->GetColorTarget(FrameBufferDiction::CombinedResult),
				pTemporalFrameBuffer->GetColorTarget(FrameBufferDiction::CombinedResult)->CreateLinearClampToEdgeSampler(),
				pTemporalFrameBuffer->GetColorTarget(FrameBufferDiction::CombinedResult)->CreateDefaultImageView()
			});
			temporalCoCResults.push_back
			({
				pTemporalFrameBuffer->GetColorTarget(FrameBufferDiction::CoC),
				pTemporalFrameBuffer->GetColorTarget(FrameBufferDiction::CoC)->CreateLinearClampToEdgeSampler(),
				pTemporalFrameBuffer->GetColorTarget(FrameBufferDiction::CoC)->CreateDefaultImageView()
			});
		}

		std::vector<CombinedImage> outputImages;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPostfilterResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::PREFILTER)[j];
			outputImages.push_back
			({
				pPostfilterResult->GetColorTarget(0),
				pPostfilterResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pPostfilterResult->GetColorTarget(0)->CreateDefaultImageView()
			});
		}

		groupNum =
		{
			(uint32_t)std::ceil((double)outputImages[0].pImage->GetImageInfo().extent.width / (double)groupSize),
			(uint32_t)std::ceil((double)outputImages[0].pImage->GetImageInfo().extent.height / (double)groupSize),
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

				CustomizedComputeMaterial::TextureUnit::BY_FRAME,

				{
					{
						false,
						0,
						VK_IMAGE_LAYOUT_UNDEFINED,
						0,

						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						outputImages[0].pImage->GetImageInfo().initialLayout,
						VK_ACCESS_SHADER_WRITE_BIT
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
		std::vector<CombinedImage> prefilterResults;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPrefilterResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::PREFILTER)[j];

			prefilterResults.push_back
			({
				pPrefilterResult->GetColorTarget(0),
				pPrefilterResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pPrefilterResult->GetColorTarget(0)->CreateDefaultImageView()
			});
		}

		std::vector<CombinedImage> outputImages;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pBlurResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::BLUR)[j];
			outputImages.push_back
			({
				pBlurResult->GetColorTarget(0),
				pBlurResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pBlurResult->GetColorTarget(0)->CreateDefaultImageView()
			});
		}

		groupNum =
		{
			(uint32_t)std::ceil((double)outputImages[0].pImage->GetImageInfo().extent.width / (double)groupSize),
			(uint32_t)std::ceil((double)outputImages[0].pImage->GetImageInfo().extent.height / (double)groupSize),
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

				CustomizedComputeMaterial::TextureUnit::BY_FRAME,

				{
					{
						false,
						0,
						VK_IMAGE_LAYOUT_UNDEFINED,
						0,

						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						outputImages[0].pImage->GetImageInfo().initialLayout,
						VK_ACCESS_SHADER_WRITE_BIT
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
		std::vector<CombinedImage> blurResults;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pBlurResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::BLUR)[j];

			blurResults.push_back
			({
				pBlurResult->GetColorTarget(0),
				pBlurResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pBlurResult->GetColorTarget(0)->CreateDefaultImageView()
			});
		}

		std::vector<CombinedImage> outputImages;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPostfilterResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::POSTFILTER)[j];
			outputImages.push_back
			({
				pPostfilterResult->GetColorTarget(0),
				pPostfilterResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pPostfilterResult->GetColorTarget(0)->CreateDefaultImageView()
			});
		}

		groupNum =
		{
			(uint32_t)std::ceil((double)outputImages[0].pImage->GetImageInfo().extent.width / (double)groupSize),
			(uint32_t)std::ceil((double)outputImages[0].pImage->GetImageInfo().extent.height / (double)groupSize),
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

				CustomizedComputeMaterial::TextureUnit::BY_FRAME,

				{
					{
						false,
						0,
						VK_IMAGE_LAYOUT_UNDEFINED,
						0,

						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						outputImages[0].pImage->GetImageInfo().initialLayout,
						VK_ACCESS_SHADER_WRITE_BIT
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
		std::vector<CombinedImage> postfilterResults;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPostfilterResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::POSTFILTER)[j];
			postfilterResults.push_back
			({
				pPostfilterResult->GetColorTarget(0),
				pPostfilterResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pPostfilterResult->GetColorTarget(0)->CreateDefaultImageView()
			});
		}

		std::vector<CombinedImage> combinedTemporalResult;
		std::vector<CombinedImage> temporalCoCResults;
		for (uint32_t j = 0; j < 2; j++)
		{
			std::shared_ptr<FrameBuffer> pTemporalResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_TemporalResolve)[j];
			combinedTemporalResult.push_back
			({
				pTemporalResult->GetColorTarget(FrameBufferDiction::CombinedResult),
				pTemporalResult->GetColorTarget(FrameBufferDiction::CombinedResult)->CreateLinearClampToEdgeSampler(),
				pTemporalResult->GetColorTarget(FrameBufferDiction::CombinedResult)->CreateDefaultImageView()
			});
			temporalCoCResults.push_back
			({
				pTemporalResult->GetColorTarget(FrameBufferDiction::CoC),
				pTemporalResult->GetColorTarget(FrameBufferDiction::CoC)->CreateLinearClampToEdgeSampler(),
				pTemporalResult->GetColorTarget(FrameBufferDiction::CoC)->CreateDefaultImageView()
			});
		}

		std::vector<CombinedImage> outputImages;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pDOFCombinedResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, (uint32_t)DOFPass::COMBINE)[j];
			outputImages.push_back
			({
				pDOFCombinedResult->GetColorTarget(0),
				pDOFCombinedResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pDOFCombinedResult->GetColorTarget(0)->CreateDefaultImageView()
			});
		}

		groupNum =
		{
			(uint32_t)std::ceil((double)outputImages[0].pImage->GetImageInfo().extent.width / (double)groupSize),
			(uint32_t)std::ceil((double)outputImages[0].pImage->GetImageInfo().extent.height / (double)groupSize),
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

				CustomizedComputeMaterial::TextureUnit::BY_NEXTPINGPONG,

				{
					{
						false,
						0,
						VK_IMAGE_LAYOUT_UNDEFINED,
						0,

						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						combinedTemporalResult[0].pImage->GetImageInfo().initialLayout,
						VK_ACCESS_SHADER_WRITE_BIT
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

				CustomizedComputeMaterial::TextureUnit::BY_NEXTPINGPONG,

				{
					{
						false,
						0,
						VK_IMAGE_LAYOUT_UNDEFINED,
						0,

						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						temporalCoCResults[0].pImage->GetImageInfo().initialLayout,
						VK_ACCESS_SHADER_WRITE_BIT
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

				CustomizedComputeMaterial::TextureUnit::BY_FRAME,

				{
					{
						false,
						0,
						VK_IMAGE_LAYOUT_UNDEFINED,
						0,

						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						outputImages[0].pImage->GetImageInfo().initialLayout,
						VK_ACCESS_SHADER_WRITE_BIT
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

	std::vector<CombinedImage> inputImgs;
	std::vector<CombinedImage> outputImgs;

	switch (bloomPass)
	{
	case BloomPass::PREFILTER:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pDOFResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, FrameBufferDiction::CombineLayer)[j];
			inputImgs.push_back
			({
				pDOFResult->GetColorTarget(0),
				pDOFResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pDOFResult->GetColorTarget(0)->CreateDefaultImageView()
			});

			std::shared_ptr<FrameBuffer> pTarget = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, iterIndex + 1)[j];
			outputImgs.push_back
			({
				pTarget->GetColorTarget(0),
				pTarget->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pTarget->GetColorTarget(0)->CreateDefaultImageView()
			});
		}
		break;
	case BloomPass::DOWNSAMPLE:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPrevBloomResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, iterIndex)[j];
			inputImgs.push_back
			({
				pPrevBloomResult->GetColorTarget(0),
				pPrevBloomResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pPrevBloomResult->GetColorTarget(0)->CreateDefaultImageView()
			});

			std::shared_ptr<FrameBuffer> pTarget = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, iterIndex + 1)[j];
			outputImgs.push_back
			({
				pTarget->GetColorTarget(0),
				pTarget->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pTarget->GetColorTarget(0)->CreateDefaultImageView()
			});
		}
		break;
	case BloomPass::UPSAMPLE:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPrevBloomResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, iterIndex + 1)[j];
			inputImgs.push_back
			({
				pPrevBloomResult->GetColorTarget(0),
				pPrevBloomResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pPrevBloomResult->GetColorTarget(0)->CreateDefaultImageView()
			});

			std::shared_ptr<FrameBuffer> pTarget = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, iterIndex)[j];
			outputImgs.push_back
			({
				pTarget->GetColorTarget(0),
				pTarget->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pTarget->GetColorTarget(0)->CreateDefaultImageView()
			});
		}
		break;
	default:
		ASSERTION(false);
		break;
	}

	Vector3ui groupNum =
	{
		(uint32_t)std::ceil((double)outputImgs[0].pImage->GetImageInfo().extent.width / (double)groupSize),
		(uint32_t)std::ceil((double)outputImgs[0].pImage->GetImageInfo().extent.height / (double)groupSize),
		1
	};

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;

	VkPipelineStageFlags srcStageFlags;
	VkImageLayout srcLayout;
	VkAccessFlags srcAccessFlags;

	VkPipelineStageFlags dstStageFlags;
	VkImageLayout dstLayout;
	VkAccessFlags dstAccessFlags;

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

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					outputImgs[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
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
	std::vector<CombinedImage> DOFResults;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pDOFResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, FrameBufferDiction::CombineLayer)[j];
		DOFResults.push_back
		({
			pDOFResult->GetColorTarget(0),
			pDOFResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pDOFResult->GetColorTarget(0)->CreateDefaultImageView()
		});
	}

	std::vector<CombinedImage> bloomTextures;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pBloomFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom)[j];
		bloomTextures.push_back
		({
			pBloomFrameBuffer->GetColorTarget(0),
			pBloomFrameBuffer->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pBloomFrameBuffer->GetColorTarget(0)->CreateDefaultImageView()
		});
	}

	std::vector<CombinedImage> combineTextures;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pCombineResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_CombineResult)[j];
		combineTextures.push_back
		({
			pCombineResult->GetColorTarget(0),
			pCombineResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pCombineResult->GetColorTarget(0)->CreateDefaultImageView()
		});
	}

	Vector3ui groupNum =
	{
		combineTextures[0].pImage->GetImageInfo().extent.width / groupSize,
		combineTextures[0].pImage->GetImageInfo().extent.height / groupSize,
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

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				{
					false,
					0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					0,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					combineTextures[0].pImage->GetImageInfo().initialLayout,
					VK_ACCESS_SHADER_WRITE_BIT
				},
				{
					false
				}
			}
		}
	);

	std::vector<uint8_t> pushConstantData;
	uint32_t dirtTextureIndex = -1;
	TransferBytesToVector(pushConstantData, &dirtTextureIndex, 0, sizeof(dirtTextureIndex));

	CustomizedComputeMaterial::Variables variables =
	{
		L"../data/shaders/combine.comp.spv",
		groupNum,
		textureUnits,
		pushConstantData
	};

	return CustomizedComputeMaterial::CreateMaterial(variables);
}