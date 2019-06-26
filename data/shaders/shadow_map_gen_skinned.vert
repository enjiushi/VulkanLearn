#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 5) in vec4 inBoneWeight;
layout (location = 6) in uint inBoneIndices;

#include "uniform_layout.sh"

void main() 
{
	int perObjectIndex = objectDataIndex[gl_DrawID].perObjectIndex;

	gl_Position = globalData.mainLightVPN * perObjectData[perObjectIndex].model * vec4(inPos.xyz, 1.0);
}
