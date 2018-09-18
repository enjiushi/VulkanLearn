#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout(push_constant) uniform PushConsts {
	layout (offset = 0) int pingpong;
} pushConsts;

layout (set = 3, binding = 2) uniform sampler2D ShadingResult[3];
layout (set = 3, binding = 3) uniform sampler2D TemporalHistory;

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outFragColor0;

void main() 
{
	int index = int(perFrameData.camDir.a);

	vec3 final = texture(TemporalHistory, inUv).rgb * 0.9f + texture(ShadingResult[index], inUv - perFrameData.cameraJitterOffset).rgb * 0.1f;

	outFragColor0 = vec4(final, 1.0);
}