#pragma once
#include "../vulkan/UniformBuffer.h"
#include "Singleton.h"
#include "../Maths/Matrix.h"

typedef struct _PerObjectVariables
{
	Matrix4f modelTransform;
}PerObjectVariables;

class PerObjectBuffer : public Singleton<PerObjectBuffer>
{
	static const uint32_t MAXIMUM_OBJECTS = 128;

public:
	std::shared_ptr<UniformBuffer> GetUniformBuffer() { return m_pUniformBuffer; }
	uint32_t AllocatePerObjectChunk();
	void FreePreObjectChunk(uint32_t index);

public:
	bool Init() override;

protected:
	// Search an used index within freed chunk, return value gives freed chunk index just after of input index
	std::pair<uint32_t, uint32_t> SearchFreeChunkIndex(uint32_t index, std::pair<uint32_t, uint32_t> range);

protected:
	std::shared_ptr<UniformBuffer>				m_pUniformBuffer;
	std::vector<std::pair<uint32_t, uint32_t>>	m_freeChunks;
};