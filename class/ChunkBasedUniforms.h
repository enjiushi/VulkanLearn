#pragma once

#include "../Maths/Matrix.h"
#include "UniformDataStorage.h"

class ChunkBasedUniforms : public UniformDataStorage
{

protected:
	static const uint32_t MAXIMUM_OBJECTS = 1024;

public:
	uint32_t AllocatePerObjectChunk();
	void FreePreObjectChunk(uint32_t index);

protected:
	bool Init(const std::shared_ptr<ChunkBasedUniforms>& pSelf, uint32_t numBytes);

	void InsertIntoFreeChunk(uint32_t index, uint32_t chunkIndex);
	void FreePreObjectChunk(uint32_t index, uint32_t start, uint32_t end);

	void UpdateUniformDataInternal() override;
	void SetDirtyInternal() override;

	virtual void UpdateDirtyChunkInternal(uint32_t index) = 0;
	virtual void SetChunkDirty(uint32_t index);

protected:
	std::vector<std::pair<uint32_t, uint32_t>>	m_freeChunks;
	uint32_t									m_perChunkBytes;
	std::vector<uint32_t>						m_dirtyChunks;
};