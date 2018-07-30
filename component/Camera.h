#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"

#undef near
#undef far

typedef struct _CameraInfo
{
	//projection related variables
	float		fov;
	float		aspect;
	float		near;
	float		far;
}CameraInfo;

class Camera : public BaseComponent
{
	DECLARE_CLASS_RTTI(Camera);

public:
	void Update() override;
	void LateUpdate() override;

	void SetFOV(float new_fov);
	void SetAspect(float new_aspect);
	void SetNearPlane(float new_near_plane);
	void SetFarPlane(float new_far_plane) { m_cameraInfo.far = new_far_plane; m_projDirty = true; }

	void SetCameraInfo(const CameraInfo& info);

	const CameraInfo GetCameraInfo() const { return m_cameraInfo; }
	const float GetFovH() const { return m_fovH; }

	const Matrix4f GetVPMatrix() const { return m_vpMatrix; }

	static std::shared_ptr<Camera> Create(const CameraInfo& info);

protected:
	bool Init(const CameraInfo& info, const std::shared_ptr<Camera>& pCamera);
	void UpdateViewMatrix();
	void UpdateProjMatrix();
	void UpdateCameraPosition();
	void UpdateCameraSupplementInfo();

protected:
	CameraInfo	m_cameraInfo;

	//Camera supplement info, no need to update by client, but to be calculated dynamically
	//make it as member variable just for convinience
	float		m_fovH;		//field of view in horizontal, required by some circumstances like character controller

	Matrix4f	m_vpMatrix;

	//float		m_viewDirty;
	float		m_projDirty;

	// Halton 2x3 for camera view frustom jittering
	float		m_jitterPattern[16 * 2];
};