#pragma once

#include "../Maths/Matrix.h"
#include "UniformDataStorage.h"

class DescriptorSet;

typedef struct _PerFrameVariables
{
	Matrix4f	viewMatrix;
	Matrix4f	viewCoordSystem;	// view coord system, i.e. viewMatrix inverse
	Matrix4f	VPN;	// vulkanNDC * view * projection
	Vector3f	cameraPosition;
	float		padding0;
	Vector3f	cameraDirection;
	float		frameIndex;
	Vector4f	eyeSpaceSize;	// xy: eye space size, zw: inverted eye space size
	Vector4f	nearFarAB;
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
	void SetCameraPosition(const Vector3f& camPos);
	Vector3f GetCameraPosition() const { return m_perFrameVariables.cameraPosition; }
	void SetCameraDirection(const Vector3f& camDir);
	Vector3f GetCameraDirection() const { return m_perFrameVariables.cameraDirection; }
	void SetEyeSpaceSize(const Vector2f& eyeSpaceSize);
	Vector2f GetEyeSpaceSize() const { return { m_perFrameVariables.eyeSpaceSize.x, m_perFrameVariables.eyeSpaceSize.y }; }
	Vector2f GetEyeSpaceSizeInv() const { return { m_perFrameVariables.eyeSpaceSize.z, m_perFrameVariables.eyeSpaceSize.w }; }
	void SetNearFarAB(const Vector4f& nearFarAB);
	Vector4f GetNearFarAB() const { return m_perFrameVariables.nearFarAB; }
	void SetPadding0(float val) { m_perFrameVariables.padding0 = val; }
	float GetPadding0() const { return m_perFrameVariables.padding0; }
	void SetFrameIndex(float frameIndex) { m_perFrameVariables.frameIndex = frameIndex; }
	float GetFrameIndex() const { return m_perFrameVariables.frameIndex; }

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void SyncBufferDataInternal() override;
	void SetDirty() override;

protected:
	PerFrameVariables	m_perFrameVariables;
};