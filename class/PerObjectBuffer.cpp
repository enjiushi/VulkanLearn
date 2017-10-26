#pragma once
#include "PerObjectBuffer.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"

bool PerObjectBuffer::Init()
{
	if (!Singleton<PerObjectBuffer>::Init())
		return false;

	m_pShaderStorageBuffer = ShaderStorageBuffer::Create(GetDevice(), MAXIMUM_OBJECTS * sizeof(PerObjectVariables) * GetSwapChain()->GetSwapChainImageCount());
	m_freeChunks.push_back({ 0, MAXIMUM_OBJECTS });
	m_frameOffset = MAXIMUM_OBJECTS * sizeof(PerObjectVariables);
	return true;
}

uint32_t PerObjectBuffer::AllocatePerObjectChunk()
{
	ASSERTION(m_freeChunks.size() != 0);

	uint32_t index = m_freeChunks[0].first++;
	if (m_freeChunks[0].first > m_freeChunks[0].second)
		m_freeChunks.erase(m_freeChunks.begin());

	return index;
}

std::pair<uint32_t, uint32_t> PerObjectBuffer::SearchFreeChunkIndex(uint32_t index, std::pair<uint32_t, uint32_t> range)
{
	if (range.first + 1 == range.second)
	{
		if (m_freeChunks[range.first].second < index < m_freeChunks[range.second].first)
			return range;
		else if (index < m_freeChunks[range.first].first)
			return std::make_pair(range.first - 1, range.first);
		else if (index > m_freeChunks[range.second].second)
			return std::make_pair(range.second, range.second + 1);
		else
			return std::make_pair(-1, -1);
	}

	uint32_t midChunkIndex = (range.first + range.second) / 2;
	
	if (m_freeChunks[midChunkIndex].first > index)
		return SearchFreeChunkIndex(index, std::make_pair(range.first, midChunkIndex));
	else
		return SearchFreeChunkIndex(index, std::make_pair(midChunkIndex, range.second));

}

void PerObjectBuffer::FreePreObjectChunk(uint32_t index)
{
	std::pair<uint32_t, uint32_t> range = SearchFreeChunkIndex(index, std::make_pair(0, m_freeChunks.size() - 1));

	// Already freed
	if (range.first == range.second)
		return;

	// If input index is smallest
	if (range.first == -1)
	{
		// See if input index can be added into later free chunk
		if (m_freeChunks[range.second].first - 1 == index)
			m_freeChunks[range.second].first--;
		// If not, insert a new free chunk
		else
			m_freeChunks.insert(m_freeChunks.begin() + range.second, { index, index });
		return;
	}

	// If input index is biggest
	if (range.second == m_freeChunks.size())
	{
		// See if input index can be added into prior free chunk
		if (m_freeChunks[range.first].second + 1 == index)
			m_freeChunks[range.first].second++;
		// If not, insert a new free chunk
		else
			m_freeChunks.insert(m_freeChunks.begin() + range.second, { index, index });
		return;
	}

	// See if input index can be added into prior free chunk
	if (m_freeChunks[range.first].second + 1 == index)
		m_freeChunks[range.first].second++;
	// See if input index can be added into later free chunk
	else if (m_freeChunks[range.second].first == index + 1)
		m_freeChunks[range.second].first--;
	// If not, insert a new free chunk
	else
		m_freeChunks.insert(m_freeChunks.begin() + range.second, { index, index });
}

void PerObjectBuffer::UpdateObjectUniformData(uint32_t index, const void* pData)
{
	m_pShaderStorageBuffer->UpdateByteStream(pData, index * sizeof(PerObjectVariables) + FrameMgr()->FrameIndex() * m_frameOffset, sizeof(PerObjectVariables));
}