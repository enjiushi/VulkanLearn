#pragma once
#include "DeviceObjectBase.h"

class Sampler : public DeviceObjectBase<Sampler>
{
public:
	~Sampler();

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Sampler>& pSelf, const VkSamplerCreateInfo& info);

public:
	static std::shared_ptr<Sampler> Create(const std::shared_ptr<Device>& pDevice, const VkSamplerCreateInfo& info);

public:
	VkSampler GetDeviceHandle() const { return m_sampler; }

protected:
	VkSampler	m_sampler;
};