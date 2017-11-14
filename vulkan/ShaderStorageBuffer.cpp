#include "ShaderStorageBuffer.h"
#include "GlobalDeviceObjects.h"
#include "SwapChain.h"

bool ShaderStorageBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<ShaderStorageBuffer>& pSelf, uint32_t numBytes)
{
	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	m_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	uint32_t minAlign = GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment;
	uint32_t alignedBytes = numBytes / minAlign * minAlign + (numBytes % minAlign > 0 ? minAlign : 0);
	uint32_t totalUniformBytes = alignedBytes * GetSwapChain()->GetSwapChainImageCount();

	m_info.size = totalUniformBytes;

	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_pBufferKey = ShaderStorageBufferMgr()->AllocateBuffer(m_info.size);
	if (!m_pBufferKey.get())
		return false;

	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	// FIXME: add them back when necessary
	m_accessStages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		//VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
		//VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
		//VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	m_accessFlags = VK_ACCESS_SHADER_READ_BIT;

	return true;
}

std::shared_ptr<ShaderStorageBuffer> ShaderStorageBuffer::Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes)
{
	std::shared_ptr<ShaderStorageBuffer> pSSBO = std::make_shared<ShaderStorageBuffer>();
	if (pSSBO.get() && pSSBO->Init(pDevice, pSSBO, numBytes))
		return pSSBO;
	return nullptr;
}

void ShaderStorageBuffer::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes)
{
	m_pBufferKey->GetSharedBufferMgr()->UpdateByteStream(pData, GetSelfSharedPtr(), m_pBufferKey, offset, numBytes);
}

uint32_t ShaderStorageBuffer::GetBufferOffset() const
{
	return m_pBufferKey->GetSharedBufferMgr()->GetOffset(m_pBufferKey);
}