#include "Sampler.h"
#include "GlobalDeviceObjects.h"

Sampler::~Sampler()
{
	if (m_sampler)
		vkDestroySampler(GetDevice()->GetDeviceHandle(), m_sampler, nullptr);
}

bool Sampler::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Sampler>& pSelf, const VkSamplerCreateInfo& info)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	RETURN_FALSE_VK_RESULT(vkCreateSampler(m_pDevice->GetDeviceHandle(), &info, nullptr, &m_sampler));

	return true;
}

std::shared_ptr<Sampler> Sampler::Create(const std::shared_ptr<Device>& pDevice, const VkSamplerCreateInfo& info)
{
	std::shared_ptr<Sampler> pSampler = std::make_shared<Sampler>();
	if (pSampler.get() && pSampler->Init(pDevice, pSampler, info))
		return pSampler;
	return nullptr;
}