#include "PlanetGeoDataManager.h"
#include "FrameEventManager.h"

bool PlanetGeoDataManager::Init()
{
	if (!Singleton<PlanetGeoDataManager>::Init())
		return false;

	// FIXME: Magic number
	m_pBufferKey = PerFrameData::GetInstance()->AllocateBuffer(4 * 1024 * 1024);

	FrameEventManager::GetInstance()->Register(m_pInstance);

	return m_pBufferKey != nullptr;
}

void* PlanetGeoDataManager::AcquireDataPtr(uint32_t& offsetInBytes) const
{
	offsetInBytes = m_updatedSize;
	return (uint8_t*)PerFrameData::GetInstance()->GetPerFrameBuffer(m_pBufferKey)->DataPtr() + m_updatedSize;
}

void PlanetGeoDataManager::FinishDataUpdate(uint32_t size)
{
	m_updatedSize += size;
	PerFrameData::GetInstance()->GetPerFrameBuffer(m_pBufferKey)->SetDirty();
}

std::shared_ptr<PerFrameBuffer> PlanetGeoDataManager::GetPerFrameBuffer() const
{
	return PerFrameData::GetInstance()->GetPerFrameBuffer(m_pBufferKey);
}

void PlanetGeoDataManager::OnFrameBegin()
{

}

void PlanetGeoDataManager::OnFrameEnd()
{
	m_updatedSize = 0;
}