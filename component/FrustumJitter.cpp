#include "FrustumJitter.h"
#include "../Base/BaseObject.h"
#include "../class/UniformData.h"
#include "Camera.h"

static uint32_t PatternLength = 32;
static float PatternScale = 1.0f;

static Vector2f Sample(float* pattern, int index)
{
	int n = PatternLength / 2;
	int i = index % n;

	return { PatternScale * pattern[2 * i + 0],  PatternScale * pattern[2 * i + 1] };
}

static float HaltonSeq(uint32_t prime, uint32_t index)
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

static void InitializeHalton_2_3(float* seq, uint32_t width, uint32_t height)
{
	for (int i = 0, n = width / 2; i != n; i++)
	{
		float u = HaltonSeq(2, i + 1) - 0.5f;
		float v = HaltonSeq(3, i + 1) - 0.5f;
		seq[2 * i + 0] = u;
		seq[2 * i + 1] = v;
	}
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

	return true;
}

void FrustumJitter::Awake()
{
	BaseComponent::Awake();
}

void FrustumJitter::Start()
{
	BaseComponent::Start();

	m_pCamera = GetComponent<Camera>();
	ASSERTION(m_pCamera != nullptr);
}