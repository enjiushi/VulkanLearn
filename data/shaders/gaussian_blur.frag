#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D InputTexture[3];

layout(push_constant) uniform PushConsts {
	layout (offset = 0) int direction;
	layout (offset = 4) float scale;
	layout (offset = 8) float strength;
} pushConsts;

layout (location = 0) in vec2 inUv;
layout (location = 0) out vec4 outFragColor0;

int index = int(perFrameData.camDir.a);

void main() 
{
	outFragColor0 = vec4(AcquireBlurredSSAO(InputTexture[index], inUv, pushConsts.direction, pushConsts.scale, pushConsts.strength), 1.0f);
}