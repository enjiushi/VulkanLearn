#include "FrustumJitter.h"
#include "../Base/BaseObject.h"
#include "../class/UniformData.h"
#include "PhysicalCamera.h"

bool HaltonSequence::Initialized = false;
uint32_t HaltonSequence::PatternLength = 32;
float HaltonSequence::PatternScale = 1.0f;

float HaltonSequence::POINTS_HALTON_2_3_X8[8 * 2];
float HaltonSequence::POINTS_HALTON_2_3_X16[16 * 2];
float HaltonSequence::POINTS_HALTON_2_3_X32[32 * 2];
float HaltonSequence::POINTS_HALTON_2_3_X256[256 * 2];

std::pair<float*, uint32_t> HaltonSequence::POINTS_HALTON_2_3[HaltonModeCount] =
{
	{ POINTS_HALTON_2_3_X8, 8 * 2 },
	{ POINTS_HALTON_2_3_X16, 16 * 2 },
	{ POINTS_HALTON_2_3_X32, 32 * 2 },
	{ POINTS_HALTON_2_3_X256, 256 * 2 }
};

Vector2f HaltonSequence::Sample(float* pattern, int index)
{
	int n = PatternLength / 2;
	int i = index % n;

	return { PatternScale * pattern[2 * i + 0],  PatternScale * pattern[2 * i + 1] };
}

float HaltonSequence::HaltonSeq(uint32_t prime, uint32_t index)
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

void HaltonSequence::InitializeHalton_2_3()
{
	if (Initialized)
		return;

	for (uint32_t mode = 0; mode < HaltonModeCount; mode++)
	{
		for (int i = 0, n = POINTS_HALTON_2_3[mode].second / 2; i != n; i++)
		{
			float u = HaltonSeq(2, i + 1) - 0.5f;
			float v = HaltonSeq(3, i + 1) - 0.5f;
			POINTS_HALTON_2_3[mode].first[2 * i + 0] = u;
			POINTS_HALTON_2_3[mode].first[2 * i + 1] = v;
		}
	}
}

Vector2f HaltonSequence::GetHaltonJitter(HaltonMode mode, uint64_t index)
{
	return { POINTS_HALTON_2_3[mode].first[index % (POINTS_HALTON_2_3[mode].second / 2)], POINTS_HALTON_2_3[mode].first[(index % (POINTS_HALTON_2_3[mode].second / 2)) + 1] };
}

DEFINITE_CLASS_RTTI(FrustumJitter, BaseComponent);

std::shared_ptr<FrustumJitter> FrustumJitter::Create()
{
	std::shared_ptr<FrustumJitter> pJitter = std::make_shared<FrustumJitter>();
	if (pJitter != nullptr && pJitter->Init(pJitter))
		return pJitter;
	return nullptr;
}

bool FrustumJitter::Init(const std::shared_ptr<FrustumJitter>& pJitter)
{
	if (!BaseComponent::Init(pJitter))
		return false;

	HaltonSequence::InitializeHalton_2_3();

	return true;
}

void FrustumJitter::Awake()
{
	BaseComponent::Awake();
}

void FrustumJitter::Start()
{
	BaseComponent::Start();

	m_pCamera = GetComponent<PhysicalCamera>();
	ASSERTION(m_pCamera != nullptr);
}

void FrustumJitter::Update()
{
	if (!m_jitterEnabled)
		return;

	m_pCamera->SetJitterOffset({ HaltonSequence::POINTS_HALTON_2_3[m_haltonMode].first[m_currentIndex], HaltonSequence::POINTS_HALTON_2_3[m_haltonMode].first[m_currentIndex + 1] });
	m_currentIndex = (m_currentIndex + 2) % HaltonSequence::POINTS_HALTON_2_3[m_haltonMode].second;
}

void FrustumJitter::SetHaltonMode(HaltonSequence::HaltonMode mode)
{
	m_haltonMode = mode;
}