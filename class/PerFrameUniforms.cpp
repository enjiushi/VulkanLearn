#pragma once
#include "PerFrameUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerFrameUniforms::Init(const std::shared_ptr<PerFrameUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(PerFrameVariables)))
		return false;
	return true;
}

std::shared_ptr<PerFrameUniforms> PerFrameUniforms::Create()
{
	std::shared_ptr<PerFrameUniforms> pPerFrameUniforms = std::make_shared<PerFrameUniforms>();
	if (pPerFrameUniforms.get() && pPerFrameUniforms->Init(pPerFrameUniforms))
		return pPerFrameUniforms;
	return nullptr;
}

void PerFrameUniforms::SetViewMatrix(const Matrix4f& viewMatrix)
{
	m_perFrameVariables.viewMatrix = viewMatrix;
	SetDirty();
}

void PerFrameUniforms::SetCameraPosition(const Vector3f& camPos)
{
	m_perFrameVariables.cameraPosition = camPos;
	SetDirty();
}

void PerFrameUniforms::SyncBufferDataInternal()
{
	GetBuffer()->UpdateByteStream(&m_perFrameVariables, FrameMgr()->FrameIndex() * sizeof(PerFrameVariables), sizeof(PerFrameVariables));
}

void PerFrameUniforms::SetDirty()
{
	m_perFrameVariables.VP = UniformData::GetInstance()->GetGlobalUniforms()->GetProjectionMatrix() * m_perFrameVariables.viewMatrix;
	m_perFrameVariables.VPN = UniformData::GetInstance()->GetGlobalUniforms()->GetNPMatrix() * m_perFrameVariables.viewMatrix;
	UniformDataStorage::SetDirty();
}

UniformVarList PerFrameUniforms::PrepareUniformVarList()
{
	return
	{
		DynamicUniformBuffer,
		"PerFrameUniforms",
		{
			{ Mat4Unit, "ViewMatrix" },
			{ Mat4Unit, "ViewProjMatrix" },
			{ Vec3Unit, "CameraPosition" }
		}
	};
}


