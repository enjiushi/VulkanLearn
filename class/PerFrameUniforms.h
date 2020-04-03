#pragma once

#include "../Maths/Matrix.h"
#include "../Maths/DualQuaternion.h"
#include "UniformDataStorage.h"
#include "ChunkBasedUniforms.h"

class DescriptorSet;

template <typename T>
class PerFrameVariables
{
public:
	Matrix4x4<T>	viewMatrix;
	Matrix4x4<T>	viewCoordSystem;
	Matrix4x4<T>	prevView;
	Matrix4x4<T>	mainLightVP;
	Vector4<T>		cameraPosition;
	Vector4<T>		cameraDeltaPosition;	// Camera position delta between 2 consecutive frames
	Vector4<T>		cameraDirection;
	Vector4<T>		eyeSpaceSize;			// xy: eye space size, zw: inverted eye space size
	Vector4<T>		nearFarAB;
	Vector4<T>		wsMainLightDir;
	Vector4<T>		mainLightDir;
	Vector4<T>		mainLightColor;
	Vector2<T>		cameraJitterOffset;
	Vector2<T>		time;					//x: delta time, y: SineTime

	Vector2<T>		haltonX8Jitter;
	Vector2<T>		haltonX16Jitter;
	Vector2<T>		haltonX32Jitter;
	Vector2<T>		haltonX256Jitter;

	T				frameIndex;
	T				pingpongIndex;
	T				padding0;
	T				padding1;
};

typedef PerFrameVariables<float> PerFrameVariablesf;
typedef PerFrameVariables<double> PerFrameVariablesd;

class PerFrameUniforms : public UniformDataStorage
{
protected:
	bool Init(const std::shared_ptr<PerFrameUniforms>& pSelf);

public:
	static std::shared_ptr<PerFrameUniforms> Create();

public:
	void SetViewMatrix(const Matrix4d& viewMatrix);
	Matrix4d GetViewMatrix() const { return m_perFrameVariables.viewMatrix; }
	void SetViewCoordinateSystem(const Matrix4d& viewCoordinateSystem);	// Maybe I should add this to reduce an extra matrix inverse
	Matrix4d GetViewCoordinateSystem() const { return m_perFrameVariables.viewCoordSystem; }
	void SetMainLightVP(const Matrix4d& vp);
	Matrix4d GetmainLightVP() const { return m_perFrameVariables.mainLightVP; }
	void SetCameraPosition(const Vector3d& camPos);
	Vector3d GetCameraPosition() const { return m_perFrameVariables.cameraPosition.xyz(); }
	void SetCameraDirection(const Vector3d& camDir);
	Vector3d GetCameraDirection() const { return m_perFrameVariables.cameraDirection.xyz(); }
	void SetEyeSpaceSize(const Vector2d& eyeSpaceSize);
	Vector2d GetEyeSpaceSize() const { return { m_perFrameVariables.eyeSpaceSize.x, m_perFrameVariables.eyeSpaceSize.y }; }
	Vector2d GetEyeSpaceSizeInv() const { return { m_perFrameVariables.eyeSpaceSize.z, m_perFrameVariables.eyeSpaceSize.w }; }
	void SetNearFarAB(const Vector4d& nearFarAB);
	Vector4d GetNearFarAB() const { return m_perFrameVariables.nearFarAB; }
	void SetWorldSpaceMainLightDir(const Vector3d& dir);
	Vector4d GetWorldSpaceMainLightDir() const { return m_perFrameVariables.wsMainLightDir; }
	void SetMainLightDir(const Vector3d& dir);
	Vector4d GetMainLightDir() const { return m_perFrameVariables.mainLightDir; }
	void SetMainLightColor(const Vector3d& color);
	Vector4d GetMainLightColor() const { return m_perFrameVariables.mainLightColor; }
	void SetCameraJitterOffset(const Vector2d& jitterOffset);
	Vector2d GetCameraJitterOffset() const { return m_perFrameVariables.cameraJitterOffset; }
	void SetDeltaTime(double deltaTime);
	double GetDeltaTime() const { return m_perFrameVariables.time.x; }
	void SetSinTime(double sinTime);
	double GetSinTime() const { return m_perFrameVariables.time.y; }
	void SetHaltonIndexX8Jitter(const Vector2d& haltonX8Jitter);
	void SetHaltonIndexX16Jitter(const Vector2d& haltonX16Jitter);
	void SetHaltonIndexX32Jitter(const Vector2d& haltonX32Jitter);
	void SetHaltonIndexX256Jitter(const Vector2d& haltonX256Jitter);
	void SetFrameIndex(double frameIndex);
	double GetFrameIndex() const { return m_perFrameVariables.frameIndex; }
	void SetPingpongIndex(double pingpongIndex);
	double GetPingpongIndex() const { return m_perFrameVariables.pingpongIndex; }
	void SetPadding0(double val);
	double GetPadding0() const { return m_perFrameVariables.padding0; }
	void SetPadding1(double val);
	double GetPadding1() const { return m_perFrameVariables.padding1; }

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void UpdateUniformDataInternal() override;
	void SetDirtyInternal() override;
	const void* AcquireDataPtr() const { return &m_singlePrecisionPerFrameVariables; }
	uint32_t AcquireDataSize() const override { return sizeof(PerFrameVariablesf); }

protected:
	PerFrameVariablesd	m_perFrameVariables;
	PerFrameVariablesf	m_singlePrecisionPerFrameVariables;
};