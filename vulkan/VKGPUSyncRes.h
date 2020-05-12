#pragma once

#include "VKFenceGuardRes.h"
#include <set>

class Semaphore;

class VKGPUSyncRes : public VKFenceGuardRes
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<VKGPUSyncRes>& pSelf);

protected:

private:
	std::set<std::shared_ptr<Semaphore>>	m_guardSemaphores;
};