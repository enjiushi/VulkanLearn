#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 inBarycentricCoord;

layout (location = 1) in vec3 inTriangleVertex;
layout (location = 2) in vec3 inTriangleEdge0;
layout (location = 3) in vec3 inTriangleEdge1;

layout (location = 0) out vec2 outUv;
layout (location = 1) out vec3 outCSNormal;
layout (location = 2) out vec3 outCSPosition;
layout (location = 3) noperspective out vec2 outScreenPosition;
layout (location = 4) out vec3 outPrevCSPosition;
layout (location = 5) out vec4 outBarycentricCoord;

#include "uniform_layout.sh"
#include "utilities.sh"

void main() 
{
	int indirectIndex = GetIndirectIndex(gl_DrawID, 0);

	int perObjectIndex = objectDataIndex[indirectIndex].perObjectIndex;

	float mixture = 0.3;
	vec2 mixBarycentric = mix(inBarycentricCoord.xy, inBarycentricCoord.zw, mixture);
	vec3 position = inTriangleVertex + inTriangleEdge0 * mixBarycentric.x + inTriangleEdge1 * mixBarycentric.y;



	vec3 planetPosition = position + perFrameData.wsCameraPosition.xyz;

	vec3 normal = normalize(planetPosition);

	float distToCamera = length(position);
	// FIXME: Remove this when I have a per-planet uniform containing planet related data including planet radius
	float radius = length(inTriangleVertex + perFrameData.wsCameraPosition.xyz);

	// Add a bias to adjust the factor
	float factor = distToCamera / (radius * globalData.PlanetRenderingSettings.x);

	// Clamp between 0 and 1
	factor = clamp(factor, 0, 1);

	// Choose to render with raw vertices or renormalized sphere vertices
	position = mix(position, normal * radius - perFrameData.wsCameraPosition.xyz, factor);

	gl_Position = perObjectData[perObjectIndex].MV_Rotation_P * vec4(position, 1.0);

	outCSNormal = normalize(mat3(perObjectData[perObjectIndex].MV) * normal);
	outCSPosition = mat3(perObjectData[perObjectIndex].MV) * position;
	outPrevCSPosition = mat3(perObjectData[perObjectIndex].prevMV) * (position + perFrameData.wsCameraDeltaPosition.xyz);
	outScreenPosition = gl_Position.xy / gl_Position.w;
	outBarycentricCoord.xy = inBarycentricCoord.xy;
	outBarycentricCoord.zw = mixBarycentric.xy;
}
