#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "../vulkan/StreamingBuffer.h"
#include "PerFrameDataStorage.h"

bool PerFrameDataStorage::Init(const std::shared_ptr<PerFrameDataStorage>& pSelf, uint32_t numBytes, StorageType storageType)
{
	if (!SelfRefBase<PerFrameDataStorage>::Init(pSelf))
		return false;

	m_pendingSync.resize(GetSwapChain()->GetSwapChainImageCount(), true);
	m_pendingSyncCount = 0;

	uint32_t minAlign = (uint32_t)GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	m_frameOffset = numBytes / minAlign * minAlign + (numBytes % minAlign > 0 ? minAlign : 0);
	uint32_t totalUniformBytes = m_frameOffset * GetSwapChain()->GetSwapChainImageCount();

	switch (storageType)
	{
	case Uniform:
		m_pBuffer = UniformBuffer::Create(GetDevice(), totalUniformBytes);
		break;
	case ShaderStorage:
		m_pBuffer = ShaderStorageBuffer::Create(GetDevice(), totalUniformBytes);
		break;
	case Streaming:
		m_pBuffer = StreamingBuffer::Create(GetDevice(), totalUniformBytes);
		break;

	default:
		ASSERTION(false);
		break;
	}

	return true;
}

void PerFrameDataStorage::SyncBufferData()
{
	if (m_pendingSyncCount == 0)
		return;

	// only update uniform data when it's just dirty
	if (m_pendingSyncCount == m_pendingSync.size())
		UpdateUniformDataInternal();

	SyncBufferDataInternal();
}

void PerFrameDataStorage::SyncBufferDataInternal()
{
	uint32_t currentFrameIndex = FrameWorkManager::GetInstance()->FrameIndex();
	if (m_pendingSync[currentFrameIndex])
		return;

	GetBuffer()->UpdateByteStream(AcquireDataPtr(), currentFrameIndex * GetFrameOffset(), AcquireDataSize());

	m_pendingSync[currentFrameIndex] = true;
	m_pendingSyncCount--;
}

void PerFrameDataStorage::SetDirty()
{
	m_pendingSyncCount = (uint32_t)m_pendingSync.size();
	for (uint32_t i = 0; i < m_pendingSyncCount; i++)
		m_pendingSync[i] = false;
	SetDirtyInternal();
}

std::shared_ptr<BufferBase> PerFrameDataStorage::GetBuffer() const
{
	return m_pBuffer;
}
