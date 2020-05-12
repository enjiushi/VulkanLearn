#include "BufferBase.h"

bool BufferBase::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<BufferBase>& pSelf, const VkBufferCreateInfo& info)
{
	if (!VKGPUSyncRes::Init(pDevice, pSelf))
		return false;
	
	m_info = info;

	return true;
}