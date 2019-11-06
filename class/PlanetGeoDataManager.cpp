#include "PlanetGeoDataManager.h"

bool PlanetGeoDataManager::Init()
{
	if (!Singleton<PlanetGeoDataManager>::Init())
		return false;

	// FIXME: Magic number
	m_pBufferKey = PerFrameData::GetInstance()->AllocateBuffer(1024 * 1024);
	return m_pBufferKey != nullptr;
}

void* PlanetGeoDataManager::AcquireDataPtr() const
{
	return (uint8_t*)PerFrameData::GetInstance()->GetPerFrameBuffer(m_pBufferKey)->DataPtr() + m_updatedSize;
}

void PlanetGeoDataManager::FinishDataUpdate(uint32_t size)
{
	//m_updatedSize += size;
	PerFrameData::GetInstance()->GetPerFrameBuffer(m_pBufferKey)->SetDirty();
}