#include "UniformBuffer.h"
#include "GlobalDeviceObjects.h"

bool UniformBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<UniformBuffer>& pSelf, uint32_t numBytes)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	uint32_t minAlign = GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	uint32_t alignedBytes = numBytes / minAlign * minAlign + (numBytes % minAlign > 0 ? minAlign : 0);

	info.size = alignedBytes;

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

std::shared_ptr<UniformBuffer> UniformBuffer::Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes)
{
	std::shared_ptr<UniformBuffer> pUniformBuffer = std::make_shared<UniformBuffer>();
	if (pUniformBuffer.get() && pUniformBuffer->Init(pDevice, pUniformBuffer, numBytes))
		return pUniformBuffer;
	return nullptr;
}

std::shared_ptr<BufferKey>	UniformBuffer::AcquireBuffer(uint32_t numBytes)
{
	return UniformBufferMgr()->AllocateBuffer(numBytes);
}