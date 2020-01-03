#include "PerFrameUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerFrameUniforms::Init(const std::shared_ptr<PerFrameUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(m_singlePrecisionPerFrameVariables), PerFrameDataStorage::Uniform))
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

void PerFrameUniforms::SetViewMatrix(const Matrix4d& viewMatrix)
{
	m_perFrameVariables.prevView = m_perFrameVariables.viewMatrix;
	m_perFrameVariables.viewMatrix = viewMatrix;
	SetDirty();
}

void PerFrameUniforms::SetViewCoordinateSystem(const Matrix4d& viewCoordinateSystem)
{
	m_perFrameVariables.viewCoordSystem = viewCoordinateSystem;
	SetDirty();
}

void PerFrameUniforms::SetCameraPosition(const Vector3d& camPos)
{
	m_perFrameVariables.cameraDeltaPosition = camPos - m_perFrameVariables.cameraPosition.xyz();
	m_perFrameVariables.cameraPosition = camPos;
	SetDirty();
}

void PerFrameUniforms::SetCameraDirection(const Vector3d& camDir)
{
	m_perFrameVariables.cameraDirection = camDir;
	SetDirty();
}

void PerFrameUniforms::SetEyeSpaceSize(const Vector2d& eyeSpaceSize)
{
	m_perFrameVariables.eyeSpaceSize = { eyeSpaceSize.x, eyeSpaceSize.y, 1.0f / eyeSpaceSize.x, 1.0f / eyeSpaceSize.y };
	SetDirty();
}

void PerFrameUniforms::SetNearFarAB(const Vector4d& nearFarAB)
{
	m_perFrameVariables.nearFarAB = nearFarAB;
	SetDirty();
}

void PerFrameUniforms::SetCameraJitterOffset(const Vector2d& jitterOffset)
{ 
	m_perFrameVariables.cameraJitterOffset = jitterOffset; 
	SetDirty();
}

void PerFrameUniforms::SetDeltaTime(double deltaTime)
{
	m_perFrameVariables.time.x = deltaTime;
	SetDirty();
}

void PerFrameUniforms::SetSinTime(double sinTime)
{
	m_perFrameVariables.time.y = sinTime;
	SetDirty();
}

void PerFrameUniforms::SetHaltonIndexX8Jitter(const Vector2d& haltonX8Jitter)
{
	m_perFrameVariables.haltonX8Jitter = haltonX8Jitter;
	SetDirty();
}

void PerFrameUniforms::SetHaltonIndexX16Jitter(const Vector2d& haltonX16Jitter)
{
	m_perFrameVariables.haltonX16Jitter = haltonX16Jitter;
	SetDirty();
}

void PerFrameUniforms::SetHaltonIndexX32Jitter(const Vector2d& haltonX32Jitter)
{
	m_perFrameVariables.haltonX32Jitter = haltonX32Jitter;
	SetDirty();
}

void PerFrameUniforms::SetHaltonIndexX256Jitter(const Vector2d& haltonX256Jitter)
{
	m_perFrameVariables.haltonX256Jitter = haltonX256Jitter;
	SetDirty();
}

void PerFrameUniforms::SetFrameIndex(double frameIndex)
{
	m_perFrameVariables.frameIndex = frameIndex;
	SetDirty();
}

void PerFrameUniforms::SetPingpongIndex(double pingpongIndex)
{
	m_perFrameVariables.pingpongIndex = pingpongIndex;
	SetDirty();
}

void PerFrameUniforms::SetPadding0(double val)
{
	m_perFrameVariables.padding0 = val;
	SetDirty();
}

void PerFrameUniforms::SetPadding1(double val)
{
	m_perFrameVariables.padding1 = val;
	SetDirty();
}


void PerFrameUniforms::UpdateUniformDataInternal()
{
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, viewMatrix);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, viewCoordSystem);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, prevView);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, cameraPosition);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, cameraDeltaPosition);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, cameraDirection);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, eyeSpaceSize);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, nearFarAB);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, cameraJitterOffset);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, time);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, haltonX8Jitter);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, haltonX16Jitter);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, haltonX32Jitter);
	CONVERT2SINGLE(m_perFrameVariables, m_singlePrecisionPerFrameVariables, haltonX256Jitter);
	CONVERT2SINGLEVAL(m_perFrameVariables, m_singlePrecisionPerFrameVariables, frameIndex);
	CONVERT2SINGLEVAL(m_perFrameVariables, m_singlePrecisionPerFrameVariables, pingpongIndex);
	CONVERT2SINGLEVAL(m_perFrameVariables, m_singlePrecisionPerFrameVariables, padding0);
	CONVERT2SINGLEVAL(m_perFrameVariables, m_singlePrecisionPerFrameVariables, padding1);
}

void PerFrameUniforms::SetDirtyInternal()
{
}

std::vector<UniformVarList> PerFrameUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicUniformBuffer,
			"PerFrameUniforms",
			{
				{ Mat4Unit, "ViewMatrix" },
				{ Mat4Unit, "ViewCoordSystem" },
				{ Mat4Unit, "prevViewMatrix" },
				{ Vec4Unit, "CameraPosition_Padding" },
				{ Vec4Unit, "CameraDeltaPosition_Padding" },
				{ Vec4Unit, "CameraDirection_FrameIndex" },
				{ Vec4Unit, "EyeSpaceSize" },
				{ Vec4Unit, "NearFarAB" },
				{ Vec2Unit, "CameraJitterOffset" },
				{ Vec2Unit, "Time, x:time, y:sin(time)" },
				{ Vec2Unit, "HaltonX8 Jitter" },
				{ Vec2Unit, "HaltonX16 Jitter" },
				{ Vec2Unit, "HaltonX32 Jitter" },
				{ Vec2Unit, "HaltonX256 Jitter" },
				{ OneUnit, "Frame Index" },
				{ OneUnit, "Pingpong Index" },
				{ OneUnit, "Reserved padding0" },
				{ OneUnit, "Reserved padding1" },
			}
		}
	};
}

uint32_t PerFrameUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateUniformBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<UniformBuffer>(GetBuffer()));

	return bindingIndex;
}

