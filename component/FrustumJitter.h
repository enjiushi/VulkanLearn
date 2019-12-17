#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../common/Singleton.h"

class PhysicalCamera;

class HaltonSequence
{
public:
	enum HaltonMode
	{
		x8,
		x16,
		x32,
		x256,
		HaltonModeCount,
	};

	static double POINTS_HALTON_2_3_X8[8 * 2];
	static double POINTS_HALTON_2_3_X16[16 * 2];
	static double POINTS_HALTON_2_3_X32[32 * 2];
	static double POINTS_HALTON_2_3_X256[256 * 2];

	static bool Initialized;
	static uint32_t PatternLength;
	static double PatternScale;

	static std::pair<double*, uint32_t> POINTS_HALTON_2_3[HaltonModeCount];

	static Vector2d Sample(double* pattern, int index);
	static double HaltonSeq(uint32_t prime, uint32_t index);
	static void InitializeHalton_2_3();
	static Vector2d GetHaltonJitter(HaltonMode mode, uint64_t index);
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
	std::shared_ptr<PhysicalCamera>	m_pCamera;
	HaltonSequence::HaltonMode		m_haltonMode = HaltonSequence::HaltonMode::x8;
	uint32_t						m_currentIndex = 0;
	bool							m_jitterEnabled = true;
};