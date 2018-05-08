#include "Util.h"
#include "Enums.h"
#include "../common/Macros.h"

uint32_t GetVertexBytes(uint32_t vertexFormat)
{
	uint32_t vertexByte = 0;
	if (vertexFormat & (1 << VAFPosition))
	{
		vertexByte += 3 * sizeof(float);
	}
	if (vertexFormat & (1 << VAFNormal))
	{
		vertexByte += 3 * sizeof(float);
	}
	if (vertexFormat & (1 << VAFColor))
	{
		vertexByte += 4 * sizeof(float);
	}
	if (vertexFormat & (1 << VAFTexCoord))
	{
		vertexByte += 2 * sizeof(float);
	}
	if (vertexFormat & (1 << VAFTangent))
	{
		vertexByte += 3 * sizeof(float);
	}
	return vertexByte;
}

uint32_t GetIndexBytes(VkIndexType indexType)
{
	switch (indexType)
	{
	case VK_INDEX_TYPE_UINT16: return 2;
	case VK_INDEX_TYPE_UINT32: return 4;
	default: ASSERTION(false);
	}
	return 0;
}

VkVertexInputBindingDescription GenerateBindingDesc(uint32_t bindingIndex, uint32_t vertexFormat)
{
	VkVertexInputBindingDescription bindingDesc = {};
	bindingDesc.binding = bindingIndex;
	bindingDesc.stride = GetVertexBytes(vertexFormat);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDesc;
}

std::vector<VkVertexInputAttributeDescription> GenerateAttribDesc(uint32_t bindingIndex, uint32_t vertexFormat)
{
	std::vector<VkVertexInputAttributeDescription> attribDesc;

	uint32_t offset = 0;
	if (vertexFormat & (1 << VAFPosition))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFPosition;
		attrib.offset = offset;
		offset += sizeof(float) * 3;
		attribDesc.push_back(attrib);
	}
	if (vertexFormat & (1 << VAFNormal))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFNormal;
		attrib.offset = offset;
		offset += sizeof(float) * 3;
		attribDesc.push_back(attrib);
	}
	if (vertexFormat & (1 << VAFColor))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attrib.location = VAFColor;
		attrib.offset = offset;
		offset += sizeof(float) * 4;
		attribDesc.push_back(attrib);
	}
	if (vertexFormat & (1 << VAFTexCoord))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFTexCoord;
		attrib.offset = offset;
		offset += sizeof(float) * 2;
		attribDesc.push_back(attrib);
	}
	if (vertexFormat & (1 << VAFTangent))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFTangent;
		attrib.offset = offset;
		offset += sizeof(float) * 3;
		attribDesc.push_back(attrib);
	}

	return attribDesc;
}