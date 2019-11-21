#include "DirectionLight.h"
#include "../Base/BaseObject.h"
#include "Camera.h"
#include <iostream>
#include <math.h>
#include "../Maths/Vector.h"
#include "../Maths/MathUtil.h"
#include "../class/UniformData.h"

DEFINITE_CLASS_RTTI(DirectionLight, BaseComponent);

bool DirectionLight::Init(const std::shared_ptr<DirectionLight>& pLight, const Vector3f& lightColor, const Vector3f& frustumSize, const Vector2ui& shadowMapSize)
{
	if (!BaseComponent::Init(pLight))
		return false;

	SetLightColor(lightColor);
	m_frustumSize = frustumSize;
	m_shadowMapSize = shadowMapSize;
	return true;
}

std::shared_ptr<DirectionLight> DirectionLight::Create(const Vector3f& lightColor, const Vector3f& frustumSize, const Vector2ui& shadowMapSize)
{
	std::shared_ptr<DirectionLight> pLight = std::make_shared<DirectionLight>();
	if (pLight.get() && pLight->Init(pLight, lightColor, frustumSize, shadowMapSize))
		return pLight;

	return nullptr;
}

Matrix4f DirectionLight::AcquireProjectionMatrix() const
{
	std::shared_ptr<BaseObject> pObj = GetBaseObject();

	Matrix4f view = pObj->GetCachedWorldTransform().Inverse();

	Matrix4f proj;
	proj.c00 = 1.0f / m_frustumSize.x;

	// Reverse y top side down for vulkan ndc
	proj.c11 = -1.0f / m_frustumSize.y;

	// -1 <= z / fz <= 1
	// 0 <= z / fz + 1 <= 2
	// 0 <= 0.5(z / fz + 1) <= 1
	// 0 <= 0.5z / fz + 0.5 <= 1
	// Convert to range 0~1 for vulkan ndc depth
	proj.c22 = 0.5f / m_frustumSize.z;
	proj.c32 = 0.5f;

	return proj * view;
}

void DirectionLight::SetLightColor(const Vector3f& lightColor)
{
	m_lightColor = lightColor;
	m_isDirty = true;
}

void DirectionLight::Update()
{
}

void DirectionLight::OnPreRender()
{
	std::shared_ptr<BaseObject> pObj = GetBaseObject();

	UniformData::GetInstance()->GetGlobalUniforms()->SetMainLightDir(pObj->GetLocalRotationM()[2]);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainLightVP(AcquireProjectionMatrix());

	if (m_isDirty)
	{
		UniformData::GetInstance()->GetGlobalUniforms()->SetMainLightColor(m_lightColor);
		m_isDirty = false;
	}
}


