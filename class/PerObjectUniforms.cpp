#pragma once
#include "PerObjectUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerObjectUniforms::Init(const std::shared_ptr<PerObjectUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(m_perObjectVariables)))
		return false;

	m_freeChunks.push_back({ 0, MAXIMUM_OBJECTS });
	return true;
}

std::shared_ptr<PerObjectUniforms> PerObjectUniforms::Create()
{
	std::shared_ptr<PerObjectUniforms> pPerObjectUniforms = std::make_shared<PerObjectUniforms>();
	if (pPerObjectUniforms.get() && pPerObjectUniforms->Init(pPerObjectUniforms))
		return pPerObjectUniforms;
	return nullptr;
}

void PerObjectUniforms::SetModelMatrix(uint32_t index, const Matrix4f& modelMatrix)
{
	m_perObjectVariables[index].modelMatrix = modelMatrix;
	SetDirty(index);
}

void PerObjectUniforms::SyncBufferDataInternal()
{
	GetBuffer()->UpdateByteStream(m_perObjectVariables, FrameMgr()->FrameIndex() * GetFrameOffset(), sizeof(m_perObjectVariables));
}

void PerObjectUniforms::SetDirty(uint32_t index)
{
	m_perObjectVariables[index].MVPN = UniformData::GetInstance()->GetPerFrameUniforms()->GetVPNMatrix() * m_perObjectVariables[index].modelMatrix;
	UniformDataStorage::SetDirty();
}

uint32_t PerObjectUniforms::AllocatePerObjectChunk()
{
	ASSERTION(m_freeChunks.size() != 0);

	uint32_t index = m_freeChunks[0].first++;
	if (m_freeChunks[0].first > m_freeChunks[0].second)
		m_freeChunks.erase(m_freeChunks.begin());

	return index;
}

std::pair<uint32_t, uint32_t> PerObjectUniforms::SearchFreeChunkIndex(uint32_t index, std::pair<uint32_t, uint32_t> range)
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

void PerObjectUniforms::FreePreObjectChunk(uint32_t index)
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

UniformVarList PerObjectUniforms::PrepareUniformVarList()
{
	return
	{
		DynamicShaderStorageBuffer,
		"PerFrameUniforms",
		{
			{ Mat4Unit, "modelMatrix" },
			{ Mat4Unit, "MVP" },
		}
	};
}


