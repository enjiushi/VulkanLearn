#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

#include "uniform_layout.sh"
#include "utilities.sh"

void main() 
{
	int perObjectIndex = objectDataIndex[GetIndirectIndex(gl_DrawID, gl_InstanceIndex)].perObjectIndex;

	gl_Position = perFrameData.mainLightVP * perObjectData[perObjectIndex].MV * vec4(inPos.xyz, 1.0);
}
