#pragma once

#include "../Maths/Matrix.h"
#include "../Maths/DualQuaternion.h"
#include "UniformDataStorage.h"
#include "ChunkBasedUniforms.h"

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

	Vector2f	haltonX8Jitter;
	Vector2f	haltonX16Jitter;
	Vector2f	haltonX32Jitter;
	Vector2f	haltonX256Jitter;
}PerFrameVariables;

typedef struct _PerFrameBoneData
{
	DualQuaternionf	boneAnimation;
}PerFramBoneData;

class PerFrameBoneUniforms : public ChunkBasedUniforms
{
protected:
	bool Init(const std::shared_ptr<PerFrameBoneUniforms>& pSelf);

public:
	static std::shared_ptr<PerFrameBoneUniforms> Create();

public:
	//void SetAnimationTransform(uint32_t index, const DualQuaternionf& animationDQ) { m_boneData[index].boneAnimation = animationDQ; SetChunkDirty(index); }

	//DualQuaternionf GetAnimationTransform(uint32_t index) const { return m_boneData[index].boneAnimation; }
	//DualQuaternionf GetReferenceTransform(uint32_t index) const { return m_boneData[index].boneReference; }

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void UpdateDirtyChunkInternal(uint32_t index) override;
	const void* AcquireDataPtr() const override { return &m_boneData[0]; }
	uint32_t AcquireDataSize() const override { return sizeof(m_boneData); }

protected:
	PerFramBoneData	m_boneData[MAXIMUM_OBJECTS];
};

class PerFrameUniforms : public UniformDataStorage
{
protected:
	bool Init(const std::shared_ptr<PerFrameUniforms>& pSelf);

public:
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
	void SetHaltonIndexX8Jitter(const Vector2f& haltonX8Jitter);
	void SetHaltonIndexX16Jitter(const Vector2f& haltonX16Jitter);
	void SetHaltonIndexX32Jitter(const Vector2f& haltonX32Jitter);
	void SetHaltonIndexX256Jitter(const Vector2f& haltonX256Jitter);

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void UpdateUniformDataInternal() override;
	void SetDirtyInternal() override;
	const void* AcquireDataPtr() const { return &m_perFrameVariables; }
	uint32_t AcquireDataSize() const override { return sizeof(PerFrameVariables); }

protected:
	PerFrameVariables	m_perFrameVariables;
};