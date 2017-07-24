#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"

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
public:
	Camera() {}
	~Camera() {}

	void Update(float delta, bool isDirty = false) override;

	void SetFOV(float new_fov);
	void SetAspect(float new_aspect);
	void SetNearPlane(float new_near_plane);
	void SetFarPlane(float new_far_plane) { m_cameraInfo.far = new_far_plane; m_projDirty = true; }

	void SetCameraInfo(const CameraInfo& info);

	const CameraInfo GetCameraInfo() const { return m_cameraInfo; }
	const float GetFovH() const { return m_fovH; }

	const Matrix4f GetViewMatrix() const { return m_viewMatrix; }
	const Matrix4f GetProjMatrix() const { return m_projMatrix; }
	const Matrix4f GetVPMatrix() const { return m_vpMatrix; }

	static Camera* Create(const CameraInfo& info);

protected:
	void UpdateViewMatrix();
	void UpdateProjMatrix();
	void UpdateVPMatrix();
	void UpdateCameraSupplementInfo();

protected:
	CameraInfo	m_cameraInfo;

	//Camera supplement info, no need to update by client, but to be calculated dynamically
	//make it as member variable just for convinience
	float		m_fovH;		//field of view in horizontal, required by some circumstances like character controller

	Matrix4f	m_viewMatrix;
	Matrix4f	m_projMatrix;
	Matrix4f	m_vpMatrix;

	float		m_viewDirty;
	float		m_projDirty;
};