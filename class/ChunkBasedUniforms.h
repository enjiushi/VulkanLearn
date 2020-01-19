#pragma once

#include "../Maths/Matrix.h"
#include "UniformDataStorage.h"

class ChunkBasedUniforms : public UniformDataStorage
{

protected:
	static const uint32_t MAXIMUM_OBJECTS = 256;

public:
	virtual uint32_t AllocatePerObjectChunk();
	virtual uint32_t AllocateConsecutiveChunks(uint32_t chunkSize);
	virtual void FreePreObjectChunk(uint32_t index);

protected:
	bool Init(const std::shared_ptr<ChunkBasedUniforms>& pSelf, uint32_t numBytes);

	void InsertIntoFreeChunk(uint32_t index, uint32_t chunkIndex);
	void FreePreObjectChunk(uint32_t index, uint32_t start, uint32_t end);

	void UpdateUniformDataInternal() override;
	void SetDirtyInternal() override;

	virtual void UpdateDirtyChunkInternal(uint32_t index) = 0;
	virtual void SetChunkDirty(uint32_t index);

	virtual void OnChunkAllocated(uint32_t index, uint32_t size) {}

protected:
	std::vector<std::pair<uint32_t, uint32_t>>	m_freeChunks;
	uint32_t									m_perChunkBytes;
	std::vector<uint32_t>						m_dirtyChunks;
};