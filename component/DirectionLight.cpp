#include "DirectionLight.h"
#include "../Base/BaseObject.h"
#include "Camera.h"
#include <iostream>
#include <math.h>
#include "../Maths/Vector.h"
#include "../Maths/MathUtil.h"
#include "../class/UniformData.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "PhysicalCamera.h"

const double DirectionLight::DEFAULT_SHADOWMAP_SIZE = 512;
const double DirectionLight::DEFAULT_FRUSTUM_SIZE = 2.56;
const double DirectionLight::DEFAULT_FRUSTUM_LENGTH = 5.12;

DEFINITE_CLASS_RTTI(DirectionLight, BaseComponent);

bool DirectionLight::Init(const std::shared_ptr<DirectionLight>& pLight, const Vector3d& lightColor, const Vector3d& frustumSize, const Vector2ui& shadowMapSize)
{
	if (!BaseComponent::Init(pLight))
		return false;

	SetLightColor(lightColor);
	m_frustumSize = frustumSize;
	m_shadowMapSize = shadowMapSize;

	m_targetLightDirectionChanged = false;

	return true;
}

std::shared_ptr<DirectionLight> DirectionLight::Create(const Vector3d& lightColor, const Vector3d& frustumSize, const Vector2ui& shadowMapSize)
{
	std::shared_ptr<DirectionLight> pLight = std::make_shared<DirectionLight>();
	if (pLight.get() && pLight->Init(pLight, lightColor, frustumSize, shadowMapSize))
	{
		InputHub::GetInstance()->Register(pLight);
		return pLight;
	}

	return nullptr;
}

void DirectionLight::UpdateData()
{
	std::shared_ptr<BaseObject> pObj = GetBaseObject();

	Matrix4d proj;
	proj.c00 = 1.0 / m_frustumSize.x;

	// Reverse y top side down for vulkan ndc
	proj.c11 = -1.0 / m_frustumSize.y;

	// -1 <= z / fz <= 1
	// 0 <= z / fz + 1 <= 2
	// 0 <= 0.5(z / fz + 1) <= 1
	// 0 <= 0.5z / fz + 0.5 <= 1
	// Convert to range 0~1 for vulkan ndc depth
	proj.c22 = 0.5 / m_frustumSize.z;
	proj.c32 = 0.5;

	// 3rd step: get light projection matrix
	m_cs2lsProjMatrix = proj;

	// light space 2 world space
	Matrix4d ls2ws = pObj->GetCachedWorldTransform();
	// light direction in world space
	m_wsLightDirection = ls2ws[2].xyz();
	// light direction in camera space
	m_csLightDirection = UniformData::GetInstance()->GetPerFrameUniforms()->GetViewMatrix().TransformAsVector(m_wsLightDirection);

	// 2nd step: from world space 2 light space
	m_cs2lsProjMatrix *= ls2ws.Inverse();

	// 1st step: from camera space 2 world space
	// final = 3rd * 2nd * 1st
	// final matrix transforms vertices from camera space 2 light space and then to light ndc
	// FIXME: should use camera world transform instead of acquiring it from per frame uniform, since it could be results from last frame
	m_cs2lsProjMatrix *= UniformData::GetInstance()->GetPerFrameUniforms()->GetViewCoordinateSystem();
}

void DirectionLight::SetLightColor(const Vector3d& lightColor)
{
	m_lightColor = lightColor;
}

void DirectionLight::Update()
{
	if (!m_targetLightDirectionChanged)
		return;

	// Set unit rotation
	std::shared_ptr<BaseObject> pObj = GetBaseObject();
	pObj->SetRotation({ 0, 0, 0, 1 });

	// Acquire transform from camera space to object space
	Matrix4d cs2os = pObj->GetWorldTransform();
	cs2os.Inverse();
	cs2os = cs2os * UniformData::GetInstance()->GetPerFrameUniforms()->GetViewCoordinateSystem();

	// Transform target light direction from camera space to object space
	m_targetLightDirectionOS = cs2os.TransformAsVector(m_targetLightDirection);

	// Acquire rotation from z axis(light direction) to target light direction
	Quaterniond rotation = Quaterniond({ 0, 0, 1 }, m_targetLightDirectionOS);
	pObj->SetRotation(rotation);
}

void DirectionLight::OnPreRender()
{
	UpdateData();

	UniformData::GetInstance()->GetPerFrameUniforms()->SetWorldSpaceMainLightDir(m_wsLightDirection);
	UniformData::GetInstance()->GetPerFrameUniforms()->SetMainLightDir(m_csLightDirection);
	UniformData::GetInstance()->GetPerFrameUniforms()->SetMainLightVP(m_cs2lsProjMatrix);
	UniformData::GetInstance()->GetPerFrameUniforms()->SetMainLightColor(m_lightColor);
}

void DirectionLight::ProcessMouse(KeyState keyState, MouseButton mouseButton, const Vector2d& mousePosition)
{
	if (mouseButton != MouseButton::LEFT)
		return;

	m_targetLightDirectionChanged = (keyState == KeyState::KEY_DOWN);
}

void DirectionLight::ProcessMouse(const Vector2d& mousePosition)
{
	if (!m_targetLightDirectionChanged)
		return;

	PrepareTargetLightDirection(mousePosition);
}

void DirectionLight::PrepareTargetLightDirection(const Vector2d& mousePosition)
{
	m_targetLightDirection = PhysicalCamera::GetViewDirectionFromScreenUV(mousePosition);
}


