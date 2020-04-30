#pragma once

#include "DeviceObjectBase.h"
#include <set>

class Fence;
class Semaphore;

class VKFenceGuardRes : public DeviceObjectBase<VKFenceGuardRes>
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<VKFenceGuardRes>& pSelf);

protected:
	void GuardByFence(const std::shared_ptr<Fence>& pFence) { m_pGuardFence = pFence; }
	void MarkOccupied() { m_isOccupied = true; }
	void WaitForRelease();
	void Release() { m_isOccupied = false; }
	bool IsOccupied() const { return m_isOccupied; }

private:
	bool	m_isOccupied;
	std::weak_ptr<Fence>	m_pGuardFence;

	friend class Fence;
};