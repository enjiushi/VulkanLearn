#include "Camera.h"
#include "../Base/BaseObject.h"
#include "../class/UniformData.h"

static uint32_t PatternLength = 32;
static float PatternScale = 1.0f;

static Vector2f Sample(float* pattern, int index)
{
	int n = PatternLength / 2;
	int i = index % n;

	return { PatternScale * pattern[2 * i + 0],  PatternScale * pattern[2 * i + 1] };
}

static float HaltonSeq(uint32_t prime, uint32_t index)
{
	float r = 0.0f;
	float f = 1.0f;
	uint32_t i = index;
	while (i > 0)
	{
		f /= prime;
		r += f * (i % prime);
		i = (uint32_t)std::floorf(i / (float)prime);
	}
	return r;
}

static void InitializeHalton_2_3(float* seq, uint32_t width, uint32_t height)
{
	for (int i = 0, n = width / 2; i != n; i++)
	{
		float u = HaltonSeq(2, i + 1) - 0.5f;
		float v = HaltonSeq(3, i + 1) - 0.5f;
		seq[2 * i + 0] = u;
		seq[2 * i + 1] = v;
	}
}

bool Camera::Init(const CameraInfo& info, const std::shared_ptr<Camera>& pCamera)
{
	if (!BaseComponent::Init(pCamera))
		return false;

	SetCameraInfo(info);

	InitializeHalton_2_3(m_jitterPattern, 16, 2);
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

	float A = -(m_cameraInfo.far + m_cameraInfo.near) / (m_cameraInfo.far - m_cameraInfo.near);
	float B = -2.0f * m_cameraInfo.near * m_cameraInfo.far / (m_cameraInfo.far - m_cameraInfo.near);

	Matrix4f proj;
	proj.x0 = 1.0f / (m_cameraInfo.aspect * tanFOV2);
	proj.y1 = 1.0f / (tanFOV2);
	proj.z2 = A;
	proj.z3 = -1.0f;
	proj.w2 = B;
	proj.w3 = 0.0f;

	float height = 2.0f * tanFOV2 * m_cameraInfo.near;
	float width = m_cameraInfo.aspect * height;

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