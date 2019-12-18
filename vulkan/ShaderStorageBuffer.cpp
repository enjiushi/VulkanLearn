#include "ShaderStorageBuffer.h"
#include "GlobalDeviceObjects.h"

bool ShaderStorageBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<ShaderStorageBuffer>& pSelf, uint32_t numBytes)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	uint32_t minAlign = (uint32_t)GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment;
	uint32_t totalUniformBytes = numBytes / minAlign * minAlign + (numBytes % minAlign > 0 ? minAlign : 0);

	info.size = totalUniformBytes;

	if (!SharedBuffer::Init(pDevice, pSelf, info))
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

std::shared_ptr<BufferKey>	ShaderStorageBuffer::AcquireBuffer(uint32_t numBytes)
{
	return ShaderStorageBufferMgr()->AllocateBuffer(numBytes);
}