#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

layout (set = 3, binding = 3) uniform sampler2D PrevBloomDownsample[3];

layout(push_constant) uniform PushConsts {
	layout (offset = 0) vec2 texelSize;
} pushConsts;

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outBloomFrag;

void main() 
{
	outBloomFrag = vec4(DownsampleBox13Tap(PrevBloomDownsample[frameIndex], inUv, pushConsts.texelSize).rgb, 1.0f);
}