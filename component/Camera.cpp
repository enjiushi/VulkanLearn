#include "Camera.h"
#include "../Base/BaseObject.h"
#include "../class/UniformData.h"

DEFINITE_CLASS_RTTI(Camera, BaseComponent);

bool Camera::Init(const CameraInfo& info, const std::shared_ptr<Camera>& pCamera)
{
	if (!BaseComponent::Init(pCamera))
		return false;

	SetCameraInfo(info);

	return true;
}

std::shared_ptr<Camera> Camera::Create(const CameraInfo& info)
{
	std::shared_ptr<Camera> pCamera = std::make_shared<Camera>();
	if (pCamera.get() && pCamera->Init(info, pCamera))
		return pCamera;
	return nullptr;
}

void Camera::Update()
{
}

void Camera::LateUpdate()
{
	UpdateCameraPosition();
	UpdateViewMatrix();
	UpdateProjMatrix();
}

void Camera::UpdateViewMatrix()
{
	if (m_pObject.expired())
		return;

	UniformData::GetInstance()->GetPerFrameUniforms()->SetViewMatrix(m_pObject.lock()->GetWorldTransform().Inverse());
	UniformData::GetInstance()->GetPerFrameUniforms()->SetCameraDirection(m_pObject.lock()->GetWorldTransform()[2].xyz().Negative());
}

void Camera::UpdateProjMatrix()
{
	if (!m_projDirty)
		return;

	float tanFOV2 = std::tanf(m_cameraInfo.fov / 2.0f);

	float A = -1.0f;
	float B = -m_cameraInfo.near;

	Matrix4f proj;

	float height = 2.0f * tanFOV2 * m_cameraInfo.near;
	float width = m_cameraInfo.aspect * height;

	// Prepare jitter offset
	//if (m_cameraInfo.jitterOffset != 0)
	{
		proj.x0 = 2.0f * m_cameraInfo.near / width;	//	2 * n / (right - left)

		proj.y1 = -2.0f * m_cameraInfo.near / height;
		// 1). x2 = ((right + jitter_offset_near_plane) + (left + jitter_offset_near_plane)) / ((right + jitter_offset_near_plane) - (left + jitter_offset_near_plane))
		// 2). x2 = (2 * jitter_offset_near_plane) / near_plane_width
		// 3). jitter_offset_near_plane = jitter_offset / window_width * near_plane_width
		// 4). x2 = 2 * jitter_offset / window_width
		// 5). jitter_offset = jitter * window_width
		// 6). x2 = 2 * jitter
		proj.z0 = 2.0f * m_cameraInfo.jitterOffset.x;
		proj.z1 = 2.0f * m_cameraInfo.jitterOffset.y;
		proj.z2 = A;
		proj.z3 = -1.0f;

		proj.w2 = B;
		proj.w3 = 0.0f;

		UniformData::GetInstance()->GetPerFrameUniforms()->SetCameraJitterOffset(m_cameraInfo.jitterOffset);
	}

	UniformData::GetInstance()->GetPerFrameUniforms()->SetEyeSpaceSize({ width, height });
	UniformData::GetInstance()->GetPerFrameUniforms()->SetNearFarAB({ m_cameraInfo.near, m_cameraInfo.far, A, B });
	UniformData::GetInstance()->GetGlobalUniforms()->SetProjectionMatrix(proj);

	m_projDirty = false;
}

void Camera::UpdateCameraPosition()
{
	UniformData::GetInstance()->GetPerFrameUniforms()->SetCameraPosition(GetBaseObject()->GetWorldPosition());
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

void Camera::SetJitterOffset(Vector2f jitterOffset)
{
	m_cameraInfo.jitterOffset = jitterOffset;
	m_cameraInfo.jitterOffset.x /= 1440.0f;
	m_cameraInfo.jitterOffset.y /= 1024.0f;
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