#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec2 outUv;

#if defined(ENABLE_CS_POS_RECONSTRUCTION)
layout (location = 1) out vec2 outOneNearPosition;
#endif

#if defined(ENABLE_CS_VIEW_RAY)
layout (location = 2) out vec3 outCSViewRay;
#endif

#include "uniform_layout.sh"

void main() 
{
	outUv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);

	gl_Position = vec4(outUv * 2.0f - 1.0f, 0.0f, 1.0f);

	// I should be careful that near is +, but it should be - in camera space
	vec3 nearPlaneCorner = vec3(perFrameData.cameraSpaceSize.xy / 2.0f * gl_Position.xy, -perFrameData.nearFarAB.x);

#if defined(ENABLE_CS_POS_RECONSTRUCTION)
	outOneNearPosition = nearPlaneCorner.xy / -nearPlaneCorner.z;
#endif

#if defined(ENABLE_CS_VIEW_RAY)
	// Camera space ray is camera position to each near plane corner
	outCSViewRay = nearPlaneCorner;
#endif

	gl_Position.y *= -1.0f;
	outUv.y = 1.0f - outUv.y;
}
