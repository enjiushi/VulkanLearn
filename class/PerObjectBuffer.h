#pragma once
#include "../vulkan/ShaderStorageBuffer.h"
#include "../common/Singleton.h"
#include "../Maths/Matrix.h"

typedef struct _PerObjectVariables
{
	Matrix4f modelTransform;
}PerObjectVariables;

class PerObjectBuffer : public Singleton<PerObjectBuffer>
{
	static const uint32_t MAXIMUM_OBJECTS = 1024;

public:
	std::shared_ptr<ShaderStorageBuffer> GetShaderStorageBuffer() { return m_pShaderStorageBuffer; }
	uint32_t AllocatePerObjectChunk();
	void FreePreObjectChunk(uint32_t index);
	uint32_t GetFrameOffset() const { return m_frameOffset; }
	void UpdateObjectUniformData(uint32_t index, const void* pData);

public:
	bool Init() override;

protected:
	// Search an used index within freed chunk, return value gives freed chunk index just after of input index
	std::pair<uint32_t, uint32_t> SearchFreeChunkIndex(uint32_t index, std::pair<uint32_t, uint32_t> range);

protected:
	std::shared_ptr<ShaderStorageBuffer>		m_pShaderStorageBuffer;
	std::vector<std::pair<uint32_t, uint32_t>>	m_freeChunks;
	uint32_t									m_frameOffset;
};