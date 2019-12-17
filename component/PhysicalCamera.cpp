#include "PhysicalCamera.h"
#include "../Base/BaseObject.h"
#include "../class/UniformData.h"

DEFINITE_CLASS_RTTI(PhysicalCamera, BaseComponent);

bool PhysicalCamera::Init(const PhysicalCameraProps& props, const std::shared_ptr<PhysicalCamera>& pCamera)
{
	if (!BaseComponent::Init(pCamera))
		return false;

	SetCameraProps(props);

	// Manually update camera properties to global uniform at init stage
	UpdateCameraProps();

	return true;
}

std::shared_ptr<PhysicalCamera> PhysicalCamera::Create(const PhysicalCameraProps& props)
{
	std::shared_ptr<PhysicalCamera> pCamera = std::make_shared<PhysicalCamera>();
	if (pCamera.get() && pCamera->Init(props, pCamera))
		return pCamera;
	return nullptr;
}

void PhysicalCamera::Update()
{
}

void PhysicalCamera::OnPreRender()
{
	UpdateCameraProps();
	UpdateViewMatrix();
	UpdateProjMatrix();
}

void PhysicalCamera::UpdateViewMatrix()
{
	if (m_pObject.expired())
		return;

	UniformData::GetInstance()->GetPerFrameUniforms()->SetCameraPosition(GetBaseObject()->GetCachedWorldPosition().DoublePrecision());
	UniformData::GetInstance()->GetPerFrameUniforms()->SetViewMatrix(m_pObject.lock()->GetCachedWorldTransform().Inverse().DoublePrecision());
	UniformData::GetInstance()->GetPerFrameUniforms()->SetCameraDirection(GetCameraDir().DoublePrecision());
}

void PhysicalCamera::UpdateProjMatrix()
{
	if (!m_projDirty)
		return;

	// Vulkan ndc depth ranges from 0 to 1
	// Depth range from near plane 0 to infinite far plane 1
	// A = f / (n - f), B = fn / (n - f)
	// When f approaches infinite
	// A = -1, B = -n
	//float A = -1;
	//float B = -m_supplementProps.fixedNearPlane;

	// Reverse depth range from 0~1 to 1~0
	// Since depth is nonlinear, and float in range 0~1 is nonlinear too
	// Both of them tends to have more accuracy towards 0
	// Reversing depth could let float nonlinearity balance depth nonlinearity
	// This method seems like to be the best approach to keep depth accuracy when using a float depth buffer
	// _z = -z + 1
	// so a identity matrix z2 = -1, w2 = 1
	// Let this matrix post multiply with projection matrix
	// Which leads to this:
	float A = 0;
	float B = m_supplementProps.fixedNearPlane;

	Matrix4f proj;

	proj.x0 = 2.0f * m_supplementProps.fixedNearPlane / m_supplementProps.fixedNearPlaneWidth;	//	2 * n / (right - left)
	// 1). x2 = ((right + jitter_offset_near_plane) + (left + jitter_offset_near_plane)) / ((right + jitter_offset_near_plane) - (left + jitter_offset_near_plane))
	// 2). x2 = (2 * jitter_offset_near_plane) / near_plane_width
	// 3). jitter_offset_near_plane = jitter_offset / window_width * near_plane_width
	// 4). x2 = 2 * jitter_offset / window_width
	// 5). jitter_offset = jitter * window_width
	// 6). x2 = 2 * jitter
	proj.z0 = 2.0f * m_jitterOffset.x;

	// Since vulkan ndc is right hand and Y axis is upside down, we need to reverse it
	proj.y1 = -2.0f * m_supplementProps.fixedNearPlane / m_supplementProps.fixedNearPlaneHeight;
	proj.z1 = 2.0f * m_jitterOffset.y;

	proj.z2 = A;
	proj.w2 = B;

	proj.z3 = -1.0f;
	proj.w3 = 0.0f;

	UniformData::GetInstance()->GetPerFrameUniforms()->SetCameraJitterOffset(m_jitterOffset.DoublePrecision());
	UniformData::GetInstance()->GetPerFrameUniforms()->SetEyeSpaceSize({ m_supplementProps.fixedNearPlaneWidth, m_supplementProps.fixedNearPlaneHeight });
	UniformData::GetInstance()->GetPerFrameUniforms()->SetNearFarAB({ m_supplementProps.fixedNearPlane, m_props.farPlane, A, B });
	UniformData::GetInstance()->GetGlobalUniforms()->SetProjectionMatrix(proj);

	m_projDirty = false;
}

void PhysicalCamera::UpdateCameraProps()
{
	if (!m_propDirty)
		return;

	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraAspect(m_props.aspect);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraFilmWidth(m_props.filmWidth);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraFilmHeight(m_supplementProps.filmHeight);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraFocalLength(m_props.focalLength);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraFocusDistance(m_props.focusDistance);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraFStop(m_props.fstop);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraShutterSpeed(m_props.shutterSpeed);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraISO(m_props.ISO);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraFarPlane(m_props.farPlane);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraHorizontalFOV(m_supplementProps.horizontalFOV_2 * 2.0f);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraVerticalFOV(m_supplementProps.verticalFOV_2 * 2.0f);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraApertureDiameter(m_supplementProps.apertureDiameter);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraHorizontalTangentFOV_2(m_supplementProps.tangentHorizontalFOV_2);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainCameraVerticalTangentFOV_2(m_supplementProps.tangentVerticalFOV_2);

	m_propDirty = false;
}

void PhysicalCamera::SetJitterOffset(Vector2f jitterOffset)
{
	m_jitterOffset = jitterOffset;
	m_jitterOffset.x /= FrameBufferDiction::WINDOW_WIDTH;
	m_jitterOffset.y /= FrameBufferDiction::WINDOW_HEIGHT;
	m_projDirty = true;
}

void PhysicalCamera::SetFilmWidth(float filmWidth)
{
	m_props.filmWidth = filmWidth;

	UpdateCameraSupplementProps();
}

void PhysicalCamera::SetFocalLength(float focalLength)
{
	m_props.focalLength = focalLength;

	UpdateCameraSupplementProps();
}

void PhysicalCamera::SetFStop(float fstop)
{
	m_props.fstop = fstop;

	UpdateCameraSupplementProps();
}

void PhysicalCamera::SetShutterSpeed(float shutterSpeed)
{
	m_props.shutterSpeed = shutterSpeed;

	m_propDirty = true;
}

void PhysicalCamera::SetISO(float ISO)
{
	m_props.ISO = ISO;

	m_propDirty = true;
}

void PhysicalCamera::SetFarPlane(float farPlane)
{
	m_props.farPlane = farPlane;

	m_projDirty = true;
	m_propDirty = true;
}

void PhysicalCamera::SetCameraProps(const PhysicalCameraProps& props)
{ 
	m_props = props;

	UpdateCameraSupplementProps();
}

void PhysicalCamera::UpdateCameraSupplementProps()
{
	m_supplementProps.filmHeight = m_props.filmWidth / m_props.aspect;
	m_supplementProps.apertureDiameter = m_props.focalLength / m_props.fstop;
	m_supplementProps.tangentHorizontalFOV_2 = m_props.filmWidth * 0.5f / m_props.focalLength;
	m_supplementProps.tangentVerticalFOV_2 = m_props.filmWidth * 0.5f / (m_props.aspect * m_props.focalLength);
	m_supplementProps.horizontalFOV_2 = std::atanf(m_supplementProps.tangentHorizontalFOV_2);
	m_supplementProps.verticalFOV_2 = std::atanf(m_supplementProps.tangentVerticalFOV_2);
	m_supplementProps.fixedNearPlaneHeight = m_supplementProps.fixedNearPlane * m_supplementProps.filmHeight / m_props.focalLength;	// 2 * n * tan(FOV_2)
	m_supplementProps.fixedNearPlaneWidth = m_supplementProps.fixedNearPlaneHeight * m_props.aspect;

	m_frustum = { {0, 0, 0}, {0, 0, -1}, m_supplementProps.verticalFOV_2, m_props.aspect };

	m_projDirty = true;
	m_propDirty = true;
}

const Vector3f PhysicalCamera::GetCameraDir() const
{
	return m_pObject.lock()->GetCachedWorldTransform()[2].xyz().Negative();
}