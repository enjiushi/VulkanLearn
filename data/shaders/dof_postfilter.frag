#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

layout (set = 3, binding = 3) uniform sampler2D DOFBlurredResult[3];

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outPostfilter;

void main() 
{
	vec4 offset = globalData.gameWindowSize.zwzw * vec4(1, 1, -1, 0.0f);
	vec4 c = texture(DOFBlurredResult[frameIndex], inUv - offset.xy);
	c += texture(DOFBlurredResult[frameIndex], inUv - offset.zy);
	c += texture(DOFBlurredResult[frameIndex], inUv + offset.zy);
	c += texture(DOFBlurredResult[frameIndex], inUv + offset.xy);
	outPostfilter = c * 0.25f;
}