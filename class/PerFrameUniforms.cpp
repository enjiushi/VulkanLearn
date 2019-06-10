#include "PerFrameUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerFrameBoneUniforms::Init(const std::shared_ptr<PerFrameBoneUniforms>& pSelf)
{
	if (!ChunkBasedUniforms::Init(pSelf, sizeof(BoneData)))
		return false;
	return true;
}

std::shared_ptr<PerFrameBoneUniforms> PerFrameBoneUniforms::Create()
{
	std::shared_ptr<PerFrameBoneUniforms> pPerFrameBoneUniforms = std::make_shared<PerFrameBoneUniforms>();
	if (pPerFrameBoneUniforms.get() && pPerFrameBoneUniforms->Init(pPerFrameBoneUniforms))
		return pPerFrameBoneUniforms;
	return nullptr;
}

void PerFrameBoneUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
}

std::vector<UniformVarList> PerFrameBoneUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"PerFrameBoneUniforms",
			{
				{ Mat2x4Unit, "Animation transform of dual quaternion" },
				{ Mat2x4Unit, "Reference transform of dual quaternion" },
			}
		}
	};
}

uint32_t PerFrameBoneUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}

bool PerFrameUniforms::Init(const std::shared_ptr<PerFrameUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(m_perFrameVariables), false))
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
	m_perFrameVariables.prevView = m_perFrameVariables.viewMatrix;
	m_perFrameVariables.viewMatrix = viewMatrix;
	m_perFrameVariables.viewCoordSystem = viewMatrix;
	m_perFrameVariables.viewCoordSystem.Inverse();
	SetDirty();
}

void PerFrameUniforms::SetCameraPosition(const Vector3f& camPos)
{
	m_perFrameVariables.cameraPosition = camPos;
	SetDirty();
}

void PerFrameUniforms::SetCameraDirection(const Vector3f& camDir)
{
	m_perFrameVariables.cameraDirection = camDir;
	SetDirty();
}

void PerFrameUniforms::SetEyeSpaceSize(const Vector2f& eyeSpaceSize)
{
	m_perFrameVariables.eyeSpaceSize = { eyeSpaceSize.x, eyeSpaceSize.y, 1.0f / eyeSpaceSize.x, 1.0f / eyeSpaceSize.y };
	SetDirty();
}

void PerFrameUniforms::SetNearFarAB(const Vector4f& nearFarAB)
{
	m_perFrameVariables.nearFarAB = nearFarAB;
	SetDirty();
}

void PerFrameUniforms::SetPadding0(float val) 
{
	m_perFrameVariables.padding0 = val; 
	SetDirty();
}

void PerFrameUniforms::SetFrameIndex(float frameIndex)
{
	m_perFrameVariables.frameIndex = frameIndex;
	SetDirty();
}

void PerFrameUniforms::SetCameraJitterOffset(const Vector2f& jitterOffset)
{ 
	m_perFrameVariables.cameraJitterOffset = jitterOffset; 
	SetDirty();
}

void PerFrameUniforms::SetDeltaTime(float deltaTime)
{
	m_perFrameVariables.time.x = deltaTime;
	SetDirty();
}

void PerFrameUniforms::SetSinTime(float sinTime)
{
	m_perFrameVariables.time.y = sinTime;
	SetDirty();
}

void PerFrameUniforms::SetHaltonIndexX8Jitter(const Vector2f& haltonX8Jitter)
{
	m_perFrameVariables.haltonX8Jitter = haltonX8Jitter;
	SetDirty();
}

void PerFrameUniforms::SetHaltonIndexX16Jitter(const Vector2f& haltonX16Jitter)
{
	m_perFrameVariables.haltonX16Jitter = haltonX16Jitter;
	SetDirty();
}

void PerFrameUniforms::SetHaltonIndexX32Jitter(const Vector2f& haltonX32Jitter)
{
	m_perFrameVariables.haltonX32Jitter = haltonX32Jitter;
	SetDirty();
}

void PerFrameUniforms::SetHaltonIndexX256Jitter(const Vector2f& haltonX256Jitter)
{
	m_perFrameVariables.haltonX256Jitter = haltonX256Jitter;
	SetDirty();
}

void PerFrameUniforms::UpdateUniformDataInternal()
{
	// Using curr proj matrix here, to get rid of camera jitter effect
	m_perFrameVariables.prevVPN = UniformData::GetInstance()->GetGlobalUniforms()->GetPNMatrix() * m_perFrameVariables.prevView;
	m_perFrameVariables.VPN = UniformData::GetInstance()->GetGlobalUniforms()->GetPNMatrix() * m_perFrameVariables.viewMatrix;
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
				{ Mat4Unit, "ViewProjMatrix" },
				{ Mat4Unit, "prevViewMatrix" },
				{ Mat4Unit, "prevVPN" },
				{ Vec4Unit, "CameraPosition_Padding" },
				{ Vec4Unit, "CameraDirection_FrameIndex" },
				{ Vec4Unit, "EyeSpaceSize" },
				{ Vec4Unit, "NearFarAB" },
				{ Vec2Unit, "CameraJitterOffset" },
				{ Vec2Unit, "Time, x:time, y:sin(time)" },
				{ Vec2Unit, "HaltonX8 Jitter" },
				{ Vec2Unit, "HaltonX16 Jitter" },
				{ Vec2Unit, "HaltonX32 Jitter" },
				{ Vec2Unit, "HaltonX256 Jitter" },
			}
		}
	};
}

uint32_t PerFrameUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateUniformBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<UniformBuffer>(GetBuffer()));

	return bindingIndex;
}

