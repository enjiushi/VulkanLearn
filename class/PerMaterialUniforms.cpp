#pragma once

#include "PerMaterialUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerMaterialUniforms::Init(const std::shared_ptr<PerMaterialUniforms>& pSelf, uint32_t numBytes)
{
	if (!ChunkBasedUniforms::Init(pSelf, numBytes))
		return false;

	m_pData = new uint8_t[GetFrameOffset()];
	memset(m_pData, 0, GetFrameOffset());
	m_perMaterialInstanceBytes = numBytes;

	return true;
}

PerMaterialUniforms::~PerMaterialUniforms()
{
	delete [] m_pData;
}

std::shared_ptr<PerMaterialUniforms> PerMaterialUniforms::Create(uint32_t numBytes)
{
	std::shared_ptr<PerMaterialUniforms> pPerMaterialUniforms = std::make_shared<PerMaterialUniforms>();
	if (pPerMaterialUniforms.get() && pPerMaterialUniforms->Init(pPerMaterialUniforms, numBytes))
		return pPerMaterialUniforms;
	return nullptr;
}

void PerMaterialUniforms::SyncBufferDataInternal()
{
	GetBuffer()->UpdateByteStream(m_pData, FrameMgr()->FrameIndex() * GetFrameOffset(), GetFrameOffset());
}

void PerMaterialUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t reservedIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(reservedIndex, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));
}
