#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D TemporalResult[2];
layout (set = 3, binding = 3) uniform sampler2D BloomTextures[3];

layout (location = 0) out vec4 outCombineResult;

layout (location = 0) in vec2 inUv;

int index = int(perFrameData.camDir.a);
int pingpong = (int(perFrameData.camPos.a) + 1) % 2;

void main() 
{
	vec3 bloom = pow(texture(BloomTextures[index], inUv).rgb * globalData.BloomSettings1.x, vec3(globalData.BloomSettings1.y));
	vec3 temporal = texture(TemporalResult[pingpong], inUv).rgb;

	outCombineResult = vec4(bloom + temporal, 1.0f);
}