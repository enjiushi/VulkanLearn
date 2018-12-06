#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../common/Singleton.h"

class Camera;

class HaltonSequence : public Singleton<HaltonSequence>
{
public:
	bool Init() override;

public:
	enum HaltonMode
	{
		x8,
		x16,
		x32,
		x256,
		HaltonModeCount,
	};

	static float POINTS_HALTON_2_3_X8[];
	static float POINTS_HALTON_2_3_X16[];
	static float POINTS_HALTON_2_3_X32[];
	static float POINTS_HALTON_2_3_X256[];

	static bool Initialized;
	static uint32_t PatternLength;
	static float PatternScale;

	static std::pair<float*, uint32_t> POINTS_HALTON_2_3[HaltonModeCount];

	static Vector2f Sample(float* pattern, int index);
	static float HaltonSeq(uint32_t prime, uint32_t index);
	static void InitializeHalton_2_3();
};

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
	void Update() override;

public:
	void SetHaltonMode(HaltonSequence::HaltonMode mode);

protected:
	std::shared_ptr<Camera>		m_pCamera;
	HaltonSequence::HaltonMode	m_haltonMode = HaltonSequence::HaltonMode::x8;
	uint32_t					m_currentIndex = 0;
	bool						m_jitterEnabled = true;
};