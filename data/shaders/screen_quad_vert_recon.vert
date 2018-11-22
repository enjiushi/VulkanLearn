#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec2 outUv;
layout (location = 1) out vec3 outViewRay;
layout (location = 2) out vec2 outOneNearPosition;

#include "uniform_layout.h"

void main() 
{
	outUv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);

	gl_Position = vec4(outUv * 2.0f - 1.0f, 0.0f, 1.0f);

	// Get eye space ray from camera to each quad vertex
	vec3 viewSpaceRay = vec3(perFrameData.eyeSpaceSize.xy / 2.0f * gl_Position.xy, -perFrameData.nearFarAB.x);	// I should be careful that near is +, but it should be - in eye space

	// Transform ray from eye space to world space
	outViewRay = (perFrameData.viewCoordSystem * vec4(viewSpaceRay, 0.0)).xyz;

	outOneNearPosition = vec2(perFrameData.eyeSpaceSize.xy / 2.0f * gl_Position.xy / -perFrameData.nearFarAB.x);

	gl_Position.y *= -1.0f;
	outUv.y = 1.0f - outUv.y;
}
