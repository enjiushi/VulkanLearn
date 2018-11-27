#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D ShadingResult[3];

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outBloomFrag;

int index = int(perFrameData.camDir.a);

void main() 
{
	outBloomFrag.rgb = texture(ShadingResult[index], inUv).rgb;
	outBloomFrag.rgb = clamp(outBloomFrag.rgb, 0.0f, 1.0f);
}