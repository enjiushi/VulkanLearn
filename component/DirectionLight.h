#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../class/InputHub.h"

class DirectionLight : public BaseComponent, public IInputListener
{
	DECLARE_CLASS_RTTI(DirectionLight);

public:
	static const double DEFAULT_SHADOWMAP_SIZE;
	static const double DEFAULT_FRUSTUM_SIZE;
	static const double DEFAULT_FRUSTUM_LENGTH;

protected:
	bool Init(const std::shared_ptr<DirectionLight>& pLight, const Vector3d& lightColor, const Vector3d& frustumSize, const Vector2ui& shadowMapSize);

public:
	static std::shared_ptr<DirectionLight> Create(const Vector3d& lightColor, const Vector3d& frustumSize = { DEFAULT_FRUSTUM_SIZE, DEFAULT_FRUSTUM_SIZE, DEFAULT_FRUSTUM_LENGTH }, const Vector2ui& shadowMapSize = { (uint32_t)DEFAULT_SHADOWMAP_SIZE, (uint32_t)DEFAULT_SHADOWMAP_SIZE });

public:
	void SetLightColor(const Vector3d& lightColor);

	void Update() override;
	void OnPreRender() override;

	// For input listener
	void ProcessKey(KeyState keyState, uint8_t keyCode) override {}
	void ProcessMouse(KeyState keyState, MouseButton mouseButton, const Vector2d& mousePosition) override;
	void ProcessMouse(const Vector2d& mousePosition) override {}

protected:
	void UpdateData();

protected:
	Vector3d	m_lightColor;
	Vector3d	m_frustumSize;
	Vector2ui	m_shadowMapSize;
	Matrix4d	m_cs2lsProjMatrix;
	Vector3d	m_wsLightDirection;
	Vector3d	m_csLightDirection;
	Vector3d	m_targetLightDirection;
	bool		m_targetLightDirectionChanged;
};