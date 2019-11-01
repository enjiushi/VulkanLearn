#include "UniformBuffer.h"
#include "SharedBuffer.h"
#include "GlobalDeviceObjects.h"
#include "SwapChain.h"

bool UniformBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<UniformBuffer>& pSelf, uint32_t numBytes)
{
	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	m_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	uint32_t minAlign = GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	uint32_t alignedBytes = numBytes / minAlign * minAlign + (numBytes % minAlign > 0 ? minAlign : 0);

	m_info.size = alignedBytes;

	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_pBufferKey = UniformBufferMgr()->AllocateBuffer(m_info.size);
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

std::shared_ptr<UniformBuffer> UniformBuffer::Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes)
{
	std::shared_ptr<UniformBuffer> pUniformBuffer = std::make_shared<UniformBuffer>();
	if (pUniformBuffer.get() && pUniformBuffer->Init(pDevice, pUniformBuffer, numBytes))
		return pUniformBuffer;
	return nullptr;
}

void UniformBuffer::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes)
{
	m_pBufferKey->GetSharedBufferMgr()->UpdateByteStream(pData, std::static_pointer_cast<SharedBuffer>(GetSelfSharedPtr()), m_pBufferKey, offset, numBytes);
}

uint32_t UniformBuffer::GetBufferOffset() const
{
	return m_pBufferKey->GetSharedBufferMgr()->GetOffset(m_pBufferKey);
}