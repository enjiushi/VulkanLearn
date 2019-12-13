#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec2 outUv;
layout (location = 1) out vec3 prevClipSpacePos;
layout (location = 2) out vec3 currClipSpacePos;

#include "uniform_layout.sh"

void main() 
{
	outUv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);

	gl_Position = vec4(outUv * 2.0f - 1.0f, 0.00001f, 1.0f);

	vec3 cameraSpacePos = vec3((perFrameData.cameraSpaceSize.xy / 2.0f) * gl_Position.xy, -perFrameData.nearFarAB.x);

	// For background, we don't take translation into consideration, as background position is considered infinite far, so that translation won't affect it no mater how large
	// Besides, we can't really use translation of both view and model matrix here for large scale scene, since they could be extremely large which makes rounding error visible
	vec3 worldSpacePos = mat3(perFrameData.viewCoordSystem) * cameraSpacePos;
	vec3 prevViewSpacePos = mat3(perFrameData.prevView) * worldSpacePos;

	prevClipSpacePos = (globalData.projection * vec4(prevViewSpacePos, 1.0f)).xyw;
	currClipSpacePos = (globalData.projection * vec4(cameraSpacePos, 1.0f)).xyw;
	gl_Position.y *= -1.0f;
	outUv.y = 1.0f - outUv.y;
}
