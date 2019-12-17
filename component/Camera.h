#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"

#undef near
#undef far

typedef struct _CameraInfo
{
	//projection related variables
	double		fov;
	double		aspect;
	double		near;
	double		far;

	Vector2d	jitterOffset;
}CameraInfo;

class Camera : public BaseComponent
{
	DECLARE_CLASS_RTTI(Camera);

public:
	void Update() override;
	void OnPreRender() override;

	void SetFOV(double new_fov);
	void SetAspect(double new_aspect);
	void SetNearPlane(double new_near_plane);
	void SetFarPlane(double new_far_plane) { m_cameraInfo.far = new_far_plane; m_projDirty = true; }
	void SetJitterOffset(Vector2d jitterOffset);

	void SetCameraInfo(const CameraInfo& info);

	const CameraInfo GetCameraInfo() const { return m_cameraInfo; }
	const double GetFovH() const { return m_fovH; }

	const Matrix4d GetVPMatrix() const { return m_vpMatrix; }

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
	double		m_fovH;		//field of view in horizontal, required by some circumstances like character controller

	Matrix4d	m_vpMatrix;

	//float		m_viewDirty;
	double		m_projDirty;

	// Halton 2x3 for camera view frustom jittering
	double		m_jitterPattern[16 * 2];
};