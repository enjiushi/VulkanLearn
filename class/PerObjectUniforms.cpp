#pragma once
#include "PerObjectUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerObjectUniforms::Init(const std::shared_ptr<PerObjectUniforms>& pSelf)
{
	if (!ChunkBasedUniforms::Init(pSelf, sizeof(PerObjectVariables)))
		return false;

	m_freeChunks.push_back({ 0, MAXIMUM_OBJECTS });
	return true;
}

std::shared_ptr<PerObjectUniforms> PerObjectUniforms::Create()
{
	std::shared_ptr<PerObjectUniforms> pPerObjectUniforms = std::make_shared<PerObjectUniforms>();
	if (pPerObjectUniforms.get() && pPerObjectUniforms->Init(pPerObjectUniforms))
		return pPerObjectUniforms;
	return nullptr;
}

void PerObjectUniforms::SetModelMatrix(uint32_t index, const Matrix4f& modelMatrix)
{
	m_perObjectVariables[index].modelMatrix = modelMatrix;
	SetDirty(index);
}

void PerObjectUniforms::SyncBufferDataInternal()
{
	GetBuffer()->UpdateByteStream(m_perObjectVariables, FrameMgr()->FrameIndex() * GetFrameOffset(), sizeof(m_perObjectVariables));
}

void PerObjectUniforms::SetDirty(uint32_t index)
{
	m_perObjectVariables[index].MVPN = UniformData::GetInstance()->GetPerFrameUniforms()->GetVPNMatrix() * m_perObjectVariables[index].modelMatrix;
	UniformDataStorage::SetDirty();
}

UniformVarList PerObjectUniforms::PrepareUniformVarList()
{
	return
	{
		DynamicShaderStorageBuffer,
		"PerFrameUniforms",
		{
			{ Mat4Unit, "modelMatrix" },
			{ Mat4Unit, "MVP" },
		}
	};
}


