#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

layout (set = 3, binding = 3) uniform sampler2D DOFPostfilterResult[3];
layout (set = 3, binding = 4) uniform sampler2D TemporalShadingResult[2];
layout (set = 3, binding = 5) uniform sampler2D TemporalCoCResult[2];

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outDOFResult;

void main() 
{
	vec4 postfilteredCoC = texture(DOFPostfilterResult[frameIndex], inUv).rgba;
	float temporalCoC = (texture(TemporalCoCResult[pingpongIndex], inUv).r * 2.0f - 1.0f) * globalData.DOFSettings0.x;
	vec4 temporalShading = texture(TemporalShadingResult[pingpongIndex], inUv);

	float farAlpha = smoothstep(globalData.gameWindowSize.z * 2.0f, globalData.gameWindowSize.z * 4.0f, temporalCoC);
	float nearAlpha = postfilteredCoC.a;

	// mix(mix(temporalShading, postfilteredCoC.rgb, farAlpha), postfilteredCoC.rgb, nearAlpha)
	vec3 color = mix(temporalShading.rgb, postfilteredCoC.rgb, farAlpha + nearAlpha - farAlpha * nearAlpha);

	outDOFResult = vec4(color, 1.0f);
}