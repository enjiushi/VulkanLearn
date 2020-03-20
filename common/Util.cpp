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
	if (vertexFormat & (1 << VAFBone))
	{
		vertexByte += 5 * sizeof(float);
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

VkVertexInputBindingDescription GenerateReservedVBBindingDesc(uint32_t vertexFormat)
{
	VkVertexInputBindingDescription bindingDesc = {};
	bindingDesc.binding = ReservedVBBindingSlot_MeshData;
	bindingDesc.stride = GetVertexBytes(vertexFormat);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDesc;
}

std::vector<VkVertexInputAttributeDescription> GenerateReservedVBAttribDesc(uint32_t vertexFormat, uint32_t vertexFormatInMem)
{
	// Do assert all bits of vertex format must exist in vertex format in memory
	ASSERTION((vertexFormat & vertexFormatInMem) == vertexFormat);

	std::vector<VkVertexInputAttributeDescription> attribDesc;

	uint32_t offset = 0;
	if (vertexFormat & (1 << VAFPosition))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = ReservedVBBindingSlot_MeshData;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFPosition;
		attrib.offset = offset;
		attribDesc.push_back(attrib);
	}
	if (vertexFormatInMem & (1 << VAFPosition))
		offset += sizeof(float) * 3;

	if (vertexFormat & (1 << VAFNormal))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = ReservedVBBindingSlot_MeshData;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFNormal;
		attrib.offset = offset;
		attribDesc.push_back(attrib);
	}
	if (vertexFormatInMem & (1 << VAFNormal))
		offset += sizeof(float) * 3;

	if (vertexFormat & (1 << VAFColor))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = ReservedVBBindingSlot_MeshData;
		attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attrib.location = VAFColor;
		attrib.offset = offset;
		attribDesc.push_back(attrib);
	}
	if (vertexFormatInMem & (1 << VAFColor))
		offset += sizeof(float) * 4;

	if (vertexFormat & (1 << VAFTexCoord))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = ReservedVBBindingSlot_MeshData;
		attrib.format = VK_FORMAT_R32G32_SFLOAT;
		attrib.location = VAFTexCoord;
		attrib.offset = offset;
		attribDesc.push_back(attrib);
	}
	if (vertexFormatInMem & (1 << VAFTexCoord))
		offset += sizeof(float) * 2;

	if (vertexFormat & (1 << VAFTangent))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = ReservedVBBindingSlot_MeshData;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFTangent;
		attrib.offset = offset;
		attribDesc.push_back(attrib);
	}
	if (vertexFormatInMem & (1 << VAFTangent))
		offset += sizeof(float) * 3;

	if (vertexFormat & (1 << VAFBone))
	{
		// Bone weight
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = ReservedVBBindingSlot_MeshData;
		attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attrib.location = VAFBone;
		attrib.offset = offset;
		attribDesc.push_back(attrib);

		// Bone index (4 bytes, 1 per index from 0 - 255)
		attrib = {};
		attrib.binding = ReservedVBBindingSlot_MeshData;
		attrib.format = VK_FORMAT_R32_UINT;
		attrib.location = VAFBone + 1;
		attrib.offset = offset + sizeof(float) * 4;
		attribDesc.push_back(attrib);
	}
	if (vertexFormatInMem & (1 << VAFBone))
		offset += sizeof(float) * 5;

	return attribDesc;
}

void TransferBytesToVector(std::vector<uint8_t>& vec, const void* pData, uint32_t numBytes)
{
	for (uint32_t i = 0; i < numBytes; i++)
	{
		vec.push_back(*((uint32_t*)(pData) + i));
	}
}