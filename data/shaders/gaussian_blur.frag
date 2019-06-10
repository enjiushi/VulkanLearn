#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

struct GaussianBlurParams
{
	int direction;
	float scale;
	float strength;
};

layout (set = 3, binding = 2) uniform sampler2D InputTexture[3];

layout(push_constant) uniform PushConsts {
	layout (offset = 0) GaussianBlurParams params;
} pushConsts;

layout (location = 0) in vec2 inUv;
layout (location = 0) out vec4 outFragColor0;

int index = int(perFrameData.camDir.a);

void main() 
{
	outFragColor0 = vec4(Blur(InputTexture[index], inUv, pushConsts.params.direction, pushConsts.params.scale, pushConsts.params.strength).rg, 0.0f, 1.0f);
}