#pragma once
#include "PerObjectUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerObjectUniforms::Init(const std::shared_ptr<PerObjectUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(PerObjectVariables)))
		return false;
	return true;
}

std::shared_ptr<PerObjectUniforms> PerObjectUniforms::Create()
{
	std::shared_ptr<PerObjectUniforms> pPerObjectUniforms = std::make_shared<PerObjectUniforms>();
	if (pPerObjectUniforms.get() && pPerObjectUniforms->Init(pPerObjectUniforms))
		return pPerObjectUniforms;
	return nullptr;
}

void PerObjectUniforms::SetModelMatrix(const Matrix4f& modelMatrix)
{
	m_perObjectVariables.modelMatrix = modelMatrix;
	m_perObjectVariables.MVP = UniformData::GetInstance()->GetGlobalUniforms()->GetProjectionMatrix() * UniformData::GetInstance()->GetPerFrameUniforms()->GetViewMatrix() * modelMatrix;
	SetDirty();
}

void PerObjectUniforms::SyncBufferDataInternal()
{
	GetBuffer()->UpdateByteStream(&m_perObjectVariables, FrameMgr()->FrameIndex() * sizeof(PerObjectVariables), sizeof(PerObjectVariables));
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


