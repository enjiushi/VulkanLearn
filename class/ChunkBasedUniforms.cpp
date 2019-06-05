#pragma once
#include "ChunkBasedUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "UniformData.h"
#include "Material.h"

bool ChunkBasedUniforms::Init(const std::shared_ptr<ChunkBasedUniforms>& pSelf, uint32_t numBytes)
{
	if (!UniformDataStorage::Init(pSelf, numBytes * MAXIMUM_OBJECTS, true))
		return false;

	m_perChunkBytes = numBytes;

	m_freeChunks.push_back({ 0, MAXIMUM_OBJECTS });
	return true;
}

uint32_t ChunkBasedUniforms::AllocatePerObjectChunk()
{
	ASSERTION(m_freeChunks.size() != 0);

	uint32_t index = m_freeChunks[0].first++;
	if (m_freeChunks[0].first > m_freeChunks[0].second)
		m_freeChunks.erase(m_freeChunks.begin());

	// To remind you when chunks are all in use(you'll have to increase the pool, statically or dynamically)
	ASSERTION(m_freeChunks.size() != 0);

	return index;
}

void ChunkBasedUniforms::FreePreObjectChunk(uint32_t index, uint32_t start, uint32_t end)
{
	uint32_t midChunkIndex = (start + end) / 2;

	std::pair<bool, bool> val;
	val = std::make_pair<bool, bool>(index <= m_freeChunks[midChunkIndex].first, index >= m_freeChunks[midChunkIndex].first);

	int32_t left, right;

	// If index is already freed
	if (val.first && val.second)
		return;
	else if (val.first)
	{
		left = midChunkIndex - 1;
		right = midChunkIndex;
	}
	else
	{
		left = midChunkIndex;
		right = midChunkIndex + 1;
	}

	if ((left < 0 || right == m_freeChunks.size()) ||
		(index > m_freeChunks[left].second) ||
		(index < m_freeChunks[right].first))
		InsertIntoFreeChunk(index, midChunkIndex);
	else
	{
		if (right == midChunkIndex)
			FreePreObjectChunk(index, start, midChunkIndex);
		else
			FreePreObjectChunk(index, midChunkIndex, end);
	}
}

void ChunkBasedUniforms::InsertIntoFreeChunk(uint32_t index, uint32_t chunkIndex)
{
	if (index < m_freeChunks[chunkIndex].first)
	{
		if (index == m_freeChunks[chunkIndex].first - 1)
			m_freeChunks[chunkIndex].first--;
		else
			m_freeChunks.insert(m_freeChunks.begin() + chunkIndex, { index, index });
	}
	else if (index > m_freeChunks[chunkIndex].second)
	{
		if (index == m_freeChunks[chunkIndex].second + 1)
			m_freeChunks[chunkIndex].second++;
		else
			m_freeChunks.insert(m_freeChunks.begin() + chunkIndex + 1, { index, index });
	}

}

void ChunkBasedUniforms::FreePreObjectChunk(uint32_t index)
{
	FreePreObjectChunk(index, 0, m_freeChunks.size() - 1);
}

void ChunkBasedUniforms::UpdateUniformDataInternal()
{
	for (auto index : m_dirtyChunks)
	{
		UpdateDirtyChunkInternal(index);
	}
	m_dirtyChunks.clear();
}

void ChunkBasedUniforms::SetDirtyInternal()
{
}

void ChunkBasedUniforms::SetChunkDirty(uint32_t index)
{
	m_dirtyChunks.push_back(index);

	UniformDataStorage::SetDirty();
}


