#include "../vulkan/SwapChain.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "UniformDataStorage.h"

bool UniformDataStorage::Init(const std::shared_ptr<UniformDataStorage>& pSelf, uint32_t numBytes, StorageType storageType)
{
	if (!PerFrameDataStorage::Init(pSelf, numBytes, storageType))
		return false;

	return true;
}
