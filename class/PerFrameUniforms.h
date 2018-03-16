#pragma once

#include "../Maths/Matrix.h"
#include "UniformDataStorage.h"

class DescriptorSet;

typedef struct _PerFrameVariables
{
	Matrix4f	viewMatrix;
	Matrix4f	VPN;	// vulkanNDC * view * projection
	Vector3f	cameraPosition;
	float		padding;
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
	void SetPadding(float val) { m_perFrameVariables.padding = val; }
	float GetPadding() const { return m_perFrameVariables.padding; }

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void SyncBufferDataInternal() override;
	void SetDirty() override;

protected:
	PerFrameVariables	m_perFrameVariables;
};