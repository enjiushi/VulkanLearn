#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "UniformDataStorage.h"

bool UniformDataStorage::Init(const std::shared_ptr<UniformDataStorage>& pSelf, uint32_t numBytes)
{
	if (!SelfRefBase<UniformDataStorage>::Init(pSelf))
		return false;

	m_pendingSyncCount = 0;

	numBytes *= GetSwapChain()->GetSwapChainImageCount();
	if (numBytes < GetDevice()->GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.maxUniformBufferRange)
		m_pUniformBuffer = UniformBuffer::Create(GetDevice(), numBytes);
	else
		m_pShaderStorageBuffer = ShaderStorageBuffer::Create(GetDevice(), numBytes);

	return true;
}

void UniformDataStorage::SyncBufferData()
{
	if (m_pendingSyncCount == 0)
		return;

	SyncBufferDataInternal();

	m_pendingSyncCount--;
}

void UniformDataStorage::SetDirty()
{
	m_pendingSyncCount = GetSwapChain()->GetSwapChainImageCount();
}

std::shared_ptr<Buffer> UniformDataStorage::GetBuffer() 
{
	if (m_pUniformBuffer)
		return m_pUniformBuffer;
	if (m_pShaderStorageBuffer)
		return m_pShaderStorageBuffer;
}
