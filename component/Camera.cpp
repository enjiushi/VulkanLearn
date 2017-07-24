#include "Camera.h"
#include "../Base/BaseObject.h"

void Camera::Update(float delta, bool isDirty)
{
	m_viewDirty = isDirty;

	UpdateViewMatrix();
	UpdateProjMatrix();
	UpdateVPMatrix();
}

void Camera::UpdateViewMatrix()
{
	if (!m_viewDirty)
		return;

	m_viewMatrix = m_object->GetWorldTransform().Inverse();
}

void Camera::UpdateProjMatrix()
{
	if (!m_projDirty)
		return;

	float tanFOV2 = std::tanf(m_cameraInfo.fov / 2.0f);

	m_projMatrix = Matrix4f();
	m_projMatrix.x0 = 1.0f / (m_cameraInfo.aspect * tanFOV2);
	m_projMatrix.y1 = 1.0f / tanFOV2;
	m_projMatrix.z2 = -(m_cameraInfo.far + m_cameraInfo.near) / (m_cameraInfo.far - m_cameraInfo.near);
	m_projMatrix.z3 = -1.0f;
	m_projMatrix.w2 = -2.0f * m_cameraInfo.near * m_cameraInfo.far / (m_cameraInfo.far - m_cameraInfo.near);
	m_projMatrix.w3 = 0.0f;
}

void Camera::UpdateVPMatrix()
{
	if (!m_viewDirty && !m_projDirty)
		return;

	m_vpMatrix = m_projMatrix * m_viewMatrix;
	m_viewDirty = m_projDirty = false;
}

void Camera::SetFOV(float new_fov)
{
	m_cameraInfo.fov = new_fov; 
	UpdateCameraSupplementInfo();

	m_projDirty = true;
}

void Camera::SetAspect(float new_aspect)
{ 
	m_cameraInfo.aspect = new_aspect; 
	UpdateCameraSupplementInfo();

	m_projDirty = true;
}

void Camera::SetNearPlane(float new_near_plane)
{ 
	m_cameraInfo.near = new_near_plane; 
	UpdateCameraSupplementInfo();

	m_projDirty = true;
}

void Camera::SetCameraInfo(const CameraInfo& info) 
{ 
	m_cameraInfo = info; 
	UpdateCameraSupplementInfo();

	m_projDirty = true; 
}

void Camera::UpdateCameraSupplementInfo()
{
	float tanFOV2 = std::tanf(m_cameraInfo.fov / 2.0f);

	//update horizontal fov
	//tan(fovh/2) = asp*tan(fov/2)       1)
	//fovh = 2*arctan(asp*tan(fov/2))    2)
	m_fovH = std::atanf(m_cameraInfo.aspect * tanFOV2) * 2.0f;
}