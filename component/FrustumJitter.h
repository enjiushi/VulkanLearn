#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"

class Camera;

class FrustumJitter : public BaseComponent
{
	DECLARE_CLASS_RTTI(FrustumJitter, BaseComponent);

public:
	static std::shared_ptr<FrustumJitter> Create();

protected:
	bool Init(const std::shared_ptr<FrustumJitter>& pJitter);

public:
	void Awake() override;
	void Start() override;

protected:
	std::shared_ptr<Camera>	m_pCamera;

	// Halton 2x3 for camera view frustom jittering
	float		m_jitterPattern[16 * 2];
};