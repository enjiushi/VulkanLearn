#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"

class DirectionLight : public BaseComponent
{
public:
	static const uint32_t DEFAULT_SHADOWMAP_SIZE = 512;
	static const uint32_t DEFAULT_FRUSTUM_SIZE = 256;
	static const uint32_t DEFAULT_FRUSTUM_LENGTH = 256;

protected:
	bool Init(const std::shared_ptr<DirectionLight>& pLight, const Vector3f& lightColor, const Vector3f& frustumSize, const Vector2ui& shadowMapSize);

public:
	static std::shared_ptr<DirectionLight> Create(const Vector3f& lightColor, const Vector3f& frustumSize = { (float)DEFAULT_FRUSTUM_SIZE, (float)DEFAULT_FRUSTUM_SIZE, (float)DEFAULT_FRUSTUM_LENGTH }, const Vector2ui& shadowMapSize = { DEFAULT_SHADOWMAP_SIZE, DEFAULT_SHADOWMAP_SIZE });

public:
	Matrix4f AcquireProjectionMatrix() const;

	void SetLightColor(const Vector3f& lightColor);

	void Update() override;
	void LateUpdate() override;

protected:
	Vector3f	m_lightColor;
	Vector3f	m_frustumSize;
	Vector2ui	m_shadowMapSize;
};