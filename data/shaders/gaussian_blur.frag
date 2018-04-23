#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D InputTexture[3];

layout(push_constant) uniform PushConsts {
	layout (offset = 0) int direction;
} pushConsts;

layout (location = 0) in vec2 inUv;
layout (location = 0) out vec4 outFragColor0;

int index = int(perFrameData.camDir.a);
const int sampleCount = 5;
const float weight[sampleCount] = 
{
	0.227027,
	0.1945946,
	0.1216216,
	0.054054,
	0.016216
};

void main() 
{
	vec2 step = vec2(1.0f) / textureSize(InputTexture[index], 0).xy;

	vec2 dir = vec2(0.0f, 1.0f);
	if (pushConsts.direction == 1)
		dir = vec2(1.0f, 0.0f);

	dir = dir * step;

	vec3 result = texture(InputTexture[index], inUv).rgb * weight[0];
	for (int i = 1; i < sampleCount; i++)
	{
		result += texture(InputTexture[index], inUv + dir * i).rgb * weight[i];
		result += texture(InputTexture[index], inUv + dir * -i).rgb * weight[i];
	}


	outFragColor0 = vec4(result, 1.0f);
}