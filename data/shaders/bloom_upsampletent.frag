#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D PrevBloomUpsample[3];

layout(push_constant) uniform PushConsts {
	layout (offset = 0) vec2 texelSize;
} pushConsts;

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outBloomFrag;

int index = int(perFrameData.camDir.a);
int pingpong = (int(perFrameData.camPos.a) + 1) % 2;

void main() 
{
	outBloomFrag = vec4(UpsampleTent(PrevBloomUpsample[index], inUv, pushConsts.texelSize, vec4(globalData.BloomSettings0.z)).rgb, 1.0f);
}