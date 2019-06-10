#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec2 outUv;

#if defined(ENABLE_WS_POS_RECONSTRUCTION)
layout (location = 1) out vec2 outOneNearPosition;
#endif

#if defined(ENABLE_WS_VIEW_RAY)
layout (location = 2) out vec3 outWsViewRay;
#endif

#include "uniform_layout.sh"

void main() 
{
	outUv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);

	gl_Position = vec4(outUv * 2.0f - 1.0f, 0.0f, 1.0f);

#if defined(ENABLE_WS_POS_RECONSTRUCTION)
	outOneNearPosition = vec2((perFrameData.eyeSpaceSize.xy / 2.0f * gl_Position.xy + perFrameData.cameraJitterOffset) / -perFrameData.nearFarAB.x);
#endif

#if defined(ENABLE_WS_VIEW_RAY)
	// Get eye space ray from camera to each quad vertex
	vec3 viewSpaceRay = vec3(perFrameData.eyeSpaceSize.xy / 2.0f * gl_Position.xy + perFrameData.cameraJitterOffset, -perFrameData.nearFarAB.x);	// I should be careful that near is +, but it should be - in eye space

	// Transform ray from eye space to world space
	outWsViewRay = (perFrameData.viewCoordSystem * vec4(viewSpaceRay, 0.0)).xyz;
#endif

	gl_Position.y *= -1.0f;
	outUv.y = 1.0f - outUv.y;
}
