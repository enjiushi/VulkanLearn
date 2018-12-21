#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D TemporalResult[2];

layout(push_constant) uniform PushConsts {
	layout (offset = 0) vec2 texelSize;
} pushConsts;

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outBloomFrag;

int index = int(perFrameData.camDir.a);
int pingpong = (int(perFrameData.camPos.a) + 1) % 2;

void main() 
{
	vec4 color = DownsampleBox13Tap(TemporalResult[pingpong], inUv, pushConsts.texelSize);
	float factor = smoothstep(globalData.BloomSettings0.x, globalData.BloomSettings0.y, Luminance(color.rgb));

	outBloomFrag = vec4(color.rgb * factor, 1.0f);
}