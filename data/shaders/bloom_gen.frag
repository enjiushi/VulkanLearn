#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"
#include "global_parameters.sh"

layout (set = 3, binding = 3) uniform sampler2D TemporalResult[2];

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outBloomFrag;

void main() 
{
	outBloomFrag.rgb = texture(TemporalResult[pingpongIndex], inUv).rgb;
	outBloomFrag.rgb = clamp(outBloomFrag.rgb, 0.0f, 1.0f);
}