#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

layout (set = 3, binding = 2) uniform sampler2D DOFResults[3];
layout (set = 3, binding = 3) uniform sampler2D BloomTextures[3];

layout(push_constant) uniform PushConsts {
	layout (offset = 0) float camDirtTexIndex;
} pushConsts;

layout (location = 0) out vec4 outCombineResult;

layout (location = 0) in vec2 inUv;

int index = int(perFrameData.camDir.a);

void main() 
{
	vec3 camDirt = vec3(1);
	if (pushConsts.camDirtTexIndex > 0.5f)
		camDirt = texture(RGBA8_1024_MIP_2DARRAY, vec3(inUv, pushConsts.camDirtTexIndex), 0.0f).rgb;

	vec3 bloom = pow(texture(BloomTextures[index], inUv).rgb * globalData.BloomSettings1.x * camDirt, vec3(globalData.BloomSettings1.y));
	vec3 temporal = texture(DOFResults[index], inUv).rgb;

	outCombineResult = vec4(bloom + temporal, 1.0f);
}