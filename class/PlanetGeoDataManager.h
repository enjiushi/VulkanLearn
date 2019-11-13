#pragma once

#include "../common/Singleton.h"
#include "FrameEventListener.h"
#include "PerFrameData.h"

class PlanetGeoDataManager : public Singleton<PlanetGeoDataManager>, public IFrameEventListener
{
public:
	bool Init();

public:
	void* AcquireDataPtr(uint32_t& offsetInBytes) const;
	void FinishDataUpdate(uint32_t size);

	std::shared_ptr<PerFrameBuffer> GetPerFrameBuffer() const;

public:
	void OnFrameBegin() override;
	void OnFrameEnd() override;

private:
	std::shared_ptr<PerFrameData::PerFrameDataKey>	m_pBufferKey;
	uint32_t										m_updatedSize = 0;
};