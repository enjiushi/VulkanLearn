#pragma once

#include "../Maths/Matrix.h"
#include "UniformDataStorage.h"

class DescriptorSet;

typedef struct _PerFrameVariables
{
	Matrix4f	viewMatrix;
	Matrix4f	viewCoordSystem;	// view coord system, i.e. viewMatrix inverse
	Matrix4f	VPN;	// vulkanNDC * view * projection
	Matrix4f	prevView;
	Matrix4f	prevVPN;
	Vector3f	cameraPosition;
	float		padding0;
	Vector3f	cameraDirection;
	float		frameIndex;
	Vector4f	eyeSpaceSize;	// xy: eye space size, zw: inverted eye space size
	Vector4f	nearFarAB;
	Vector2f	cameraJitterOffset;
	Vector2f	time;		//x: delta time, y: SineTime
	Vector4f	haltonIndex;	// x: x8, y: x16, z: x32, w: x256
}PerFrameVariables;

class PerFrameUniforms : public UniformDataStorage
{
	static const uint32_t MAXIMUM_OBJECTS = 1024;

public:
	bool Init(const std::shared_ptr<PerFrameUniforms>& pSelf);
	static std::shared_ptr<PerFrameUniforms> Create();

public:
	void SetViewMatrix(const Matrix4f& viewMatrix);
	Matrix4f GetViewMatrix() const { return m_perFrameVariables.viewMatrix; }
	Matrix4f GetVPNMatrix() const { return m_perFrameVariables.VPN; }
	Matrix4f GetPrevViewMatrix() const { return m_perFrameVariables.prevView; }
	Matrix4f GetPrevVPNMatrix() const { return m_perFrameVariables.prevVPN; }
	void SetCameraPosition(const Vector3f& camPos);
	Vector3f GetCameraPosition() const { return m_perFrameVariables.cameraPosition; }
	void SetCameraDirection(const Vector3f& camDir);
	Vector3f GetCameraDirection() const { return m_perFrameVariables.cameraDirection; }
	void SetEyeSpaceSize(const Vector2f& eyeSpaceSize);
	Vector2f GetEyeSpaceSize() const { return { m_perFrameVariables.eyeSpaceSize.x, m_perFrameVariables.eyeSpaceSize.y }; }
	Vector2f GetEyeSpaceSizeInv() const { return { m_perFrameVariables.eyeSpaceSize.z, m_perFrameVariables.eyeSpaceSize.w }; }
	void SetNearFarAB(const Vector4f& nearFarAB);
	Vector4f GetNearFarAB() const { return m_perFrameVariables.nearFarAB; }
	void SetPadding0(float val);
	float GetPadding0() const { return m_perFrameVariables.padding0; }
	void SetFrameIndex(float frameIndex);
	float GetFrameIndex() const { return m_perFrameVariables.frameIndex; }
	void SetCameraJitterOffset(const Vector2f& jitterOffset);
	Vector2f GetCameraJitterOffset() const { return m_perFrameVariables.cameraJitterOffset; }
	void SetDeltaTime(float deltaTime);
	float GetDeltaTime() const { return m_perFrameVariables.time.x; }
	void SetSinTime(float sinTime);
	float GetSinTime() const { return m_perFrameVariables.time.y; }
	void SetHaltonIndex(uint32_t frameCount);
	Vector4f GetHaltonIndex() const { return m_perFrameVariables.haltonIndex; }

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void SyncBufferDataInternal() override;
	void SetDirty() override;

protected:
	PerFrameVariables	m_perFrameVariables;
};