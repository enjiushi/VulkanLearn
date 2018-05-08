#pragma once
#include <vulkan.h>
#include <vector>

enum VertexAttribFlag
{
	VAFPosition,
	VAFNormal,
	VAFColor,
	VAFTexCoord,
	VAFTangent,
	VACount
};

uint32_t GetVertexBytes(uint32_t vertexFormat);
uint32_t GetIndexBytes(VkIndexType indexType);
VkVertexInputBindingDescription GenerateBindingDesc(uint32_t bindingIndex, uint32_t vertexFormat);
std::vector<VkVertexInputAttributeDescription> GenerateAttribDesc(uint32_t bindingIndex, uint32_t vertexFormat);