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

	m_pendingSyncCount = 0;

	uint32_t minAlign = GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
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
	if (m_pendingSyncCount == GetSwapChain()->GetSwapChainImageCount())
		UpdateUniformDataInternal();

	SyncBufferDataInternal();

	m_pendingSyncCount--;
}

void PerFrameDataStorage::SyncBufferDataInternal()
{
	GetBuffer()->UpdateByteStream(AcquireDataPtr(), FrameMgr()->FrameIndex() * GetFrameOffset(), AcquireDataSize());
}

void PerFrameDataStorage::SetDirty()
{
	m_pendingSyncCount = GetSwapChain()->GetSwapChainImageCount();
	SetDirtyInternal();
}

std::shared_ptr<BufferBase> PerFrameDataStorage::GetBuffer() const
{
	return m_pBuffer;
}
