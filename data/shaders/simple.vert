#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 3) in vec2 inUv;
layout (location = 0) out vec2 outUv;

#include "uniform_layout.sh"

void main() 
{
	gl_Position = perObjectData[GetIndirectIndex(gl_DrawID, gl_InstanceIndex)].MVP * vec4(inPos.xyz, 1.0);

	outUv = inUv.st;
	outUv.t = 1.0 - outUv.t;
}
