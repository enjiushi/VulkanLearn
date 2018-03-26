#pragma once
#include "GBufferInputUniforms.h"
#include "Material.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/Image.h"
#include "../vulkan/Texture2D.h"
#include "../vulkan/DepthStencilBuffer.h"
#include "RenderWorkManager.h"

bool GBufferInputUniforms::Init(const std::shared_ptr<GBufferInputUniforms>& pSelf)
{
	if (!UniformBase::Init(pSelf))
		return false;

	return true;
}

std::shared_ptr<GBufferInputUniforms> GBufferInputUniforms::Create()
{
	std::shared_ptr<GBufferInputUniforms> pGBufferInputUniforms = std::make_shared<GBufferInputUniforms>();
	if (pGBufferInputUniforms.get() && pGBufferInputUniforms->Init(pGBufferInputUniforms))
		return pGBufferInputUniforms;
	return nullptr;
}

std::vector<UniformVarList> GBufferInputUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			InputAttachment,
			"RGBA8_GBuffer0"
		},
		{
			InputAttachment,
			"RGBA8_GBuffer1"
		},
		{
			InputAttachment,
			"RGBA8_GBuffer2"
		}
	};
}

uint32_t GBufferInputUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	std::vector<std::shared_ptr<Texture2D>> gbuffers = RenderWorkManager::GetInstance()->GetGBuffers();
	std::shared_ptr<DepthStencilBuffer> pDSBuffer = RenderWorkManager::GetInstance()->GetDeferredDepthStencilBuffer();

	for (uint32_t i = 0; i < gbuffers.size(); i++)
	{
		pDescriptorSet->UpdateInputImage(bindingIndex++, 
			std::static_pointer_cast<Image>(gbuffers[i]), 
			gbuffers[i]->CreateLinearClampToEdgeSampler(), 
			gbuffers[i]->CreateDefaultImageView());
	}

	pDescriptorSet->UpdateInputImage(bindingIndex++, std::static_pointer_cast<Image>(pDSBuffer), 
		pDSBuffer->CreateLinearClampToEdgeSampler(), 
		pDSBuffer->CreateDepthSampleImageView());

	return bindingIndex;
}

