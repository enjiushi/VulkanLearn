#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 prevClipSpacePos;
layout (location = 2) in vec3 currClipSpacePos;

layout (location = 0) out vec4 outFragColor;

#include "uniform_layout.h"

void main() 
{
	vec2 prevScreenCoord = prevClipSpacePos.xy / prevClipSpacePos.z;
	prevScreenCoord = prevScreenCoord * 0.5f + 0.5f;

	vec2 currScreenCoord = currClipSpacePos.xy / currClipSpacePos.z;
	currScreenCoord = currScreenCoord * 0.5f + 0.5f;

	outFragColor = vec4(prevScreenCoord - currScreenCoord, 0.0f, 1.0f);
}