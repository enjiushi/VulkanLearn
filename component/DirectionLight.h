#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"

class DirectionLight : public BaseComponent
{
	DECLARE_CLASS_RTTI(DirectionLight);

public:
	static const uint32_t DEFAULT_SHADOWMAP_SIZE = 512;
	static const uint32_t DEFAULT_FRUSTUM_SIZE = 2.56;
	static const uint32_t DEFAULT_FRUSTUM_LENGTH = 5.12;

protected:
	bool Init(const std::shared_ptr<DirectionLight>& pLight, const Vector3d& lightColor, const Vector3d& frustumSize, const Vector2ui& shadowMapSize);

public:
	static std::shared_ptr<DirectionLight> Create(const Vector3d& lightColor, const Vector3d& frustumSize = { (float)DEFAULT_FRUSTUM_SIZE, (float)DEFAULT_FRUSTUM_SIZE, (float)DEFAULT_FRUSTUM_LENGTH }, const Vector2ui& shadowMapSize = { DEFAULT_SHADOWMAP_SIZE, DEFAULT_SHADOWMAP_SIZE });

public:
	Matrix4d AcquireProjectionMatrix() const;

	void SetLightColor(const Vector3d& lightColor);

	void Update() override;
	void OnPreRender() override;

protected:
	Vector3d	m_lightColor;
	Vector3d	m_frustumSize;
	Vector2ui	m_shadowMapSize;
};