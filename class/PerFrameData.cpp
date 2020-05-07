#include "PerFrameData.h"
#include "FrameEventManager.h"

std::shared_ptr<PerFrameBuffer> PerFrameBuffer::Create(uint32_t size)
{
	std::shared_ptr<PerFrameBuffer> pPerFrameBuffer = std::make_shared<PerFrameBuffer>();
	if (pPerFrameBuffer.get() && pPerFrameBuffer->Init(pPerFrameBuffer, size))
		return pPerFrameBuffer;
	return nullptr;
}

bool PerFrameBuffer::Init(const std::shared_ptr<PerFrameBuffer>& pSelf, uint32_t size)
{
	if (!PerFrameDataStorage::Init(pSelf, size, PerFrameDataStorage::Streaming))
		return false;

	m_pData = new uint8_t[GetFrameOffset()];

	return true;
}

PerFrameBuffer::~PerFrameBuffer()
{
	if (m_pData)
	{
		delete[] m_pData;
		m_pData = nullptr;
	}
}

void PerFrameBuffer::SetDirty()
{
	PerFrameDataStorage::SetDirty();
}

PerFrameData::PerFrameDataKey::~PerFrameDataKey()
{
	m_pPerFrameData->DeallocateBuffer(key);
}

bool PerFrameData::Init()
{
	FrameEventManager::GetInstance()->Register(m_pInstance);

	return true;
}

void PerFrameData::OnPostSceneTraversal()
{
	SyncDataBuffer();
}

void PerFrameData::SyncDataBuffer()
{
	for (auto& var : m_storageBuffers)
		var->SyncBufferData();
}

std::shared_ptr<PerFrameData::PerFrameDataKey> PerFrameData::AllocateBuffer(uint32_t size)
{
	std::shared_ptr<PerFrameBuffer> pPerFrameBuffer = PerFrameBuffer::Create(size);
	if (pPerFrameBuffer == nullptr)
		return nullptr;

	m_storageBuffers.push_back(pPerFrameBuffer);
	return std::make_shared<PerFrameData::PerFrameDataKey>((uint32_t)m_storageBuffers.size() - 1, GetSharedInstance());
}

void PerFrameData::DeallocateBuffer(uint32_t key)
{
	m_storageBuffers.erase(m_storageBuffers.begin() + key);
}