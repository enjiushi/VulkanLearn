#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "UniformDataStorage.h"

bool UniformDataStorage::Init(const std::shared_ptr<UniformDataStorage>& pSelf, uint32_t numBytes, bool perObject)
{
	if (!UniformBase::Init(pSelf))
		return false;

	m_pendingSyncCount = 0;

	uint32_t minAlign = GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	m_frameOffset = numBytes / minAlign * minAlign + (numBytes % minAlign > 0 ? minAlign : 0);
	uint32_t totalUniformBytes = m_frameOffset * GetSwapChain()->GetSwapChainImageCount();

	if (!perObject)
		m_pUniformBuffer = UniformBuffer::Create(GetDevice(), totalUniformBytes);
	else
		m_pShaderStorageBuffer = ShaderStorageBuffer::Create(GetDevice(), totalUniformBytes);

	return true;
}

void UniformDataStorage::SyncBufferData()
{
	if (m_pendingSyncCount == 0)
		return;

	// only update uniform data when it's just dirty
	if (m_pendingSyncCount == GetSwapChain()->GetSwapChainImageCount())
		UpdateUniformDataInternal();

	SyncBufferDataInternal();

	m_pendingSyncCount--;
}

void UniformDataStorage::SyncBufferDataInternal()
{
	GetBuffer()->UpdateByteStream(AcquireDataPtr(), FrameMgr()->FrameIndex() * GetFrameOffset(), AcquireDataSize());
}

void UniformDataStorage::SetDirty()
{
	m_pendingSyncCount = GetSwapChain()->GetSwapChainImageCount();
	SetDirtyInternal();
}

std::shared_ptr<Buffer> UniformDataStorage::GetBuffer() const
{
	if (m_pUniformBuffer)
		return m_pUniformBuffer;
	if (m_pShaderStorageBuffer)
		return m_pShaderStorageBuffer;

	ASSERTION(false);
	return nullptr;
}
