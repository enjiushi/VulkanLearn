#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 inBarycentricCoord;

layout (location = 1) in vec3 inTriangleVertex;
layout (location = 2) in vec3 inTriangleEdge0;
layout (location = 3) in vec3 inTriangleEdge1;
layout (location = 4) in float inLevel;

layout (location = 0) out vec2 outUv;
layout (location = 1) out vec3 outCSNormal;
layout (location = 2) out vec3 outCSPosition;
layout (location = 3) noperspective out vec2 outScreenPosition;
layout (location = 4) out vec3 outPrevCSPosition;
layout (location = 5) out vec4 outBarycentricCoord;
layout (location = 6) out vec4 outSomething;

#include "uniform_layout.sh"
#include "utilities.sh"

void main() 
{
	int indirectIndex = GetIndirectIndex(gl_DrawID, 0);

	int perObjectIndex = objectDataIndex[indirectIndex].perObjectIndex;

	// Vector from morphing ending position to morphing start position
	vec2 morphEnd2Start = inBarycentricCoord.zw - inBarycentricCoord.xy;

	// Morphing start position depends on the sign(level), which is a reverse flag indicating whether morphing direction should be reversed.
	// If the flag is negative, we reverse morphing direction
	vec2 morphStart = inBarycentricCoord.xy + sign(inLevel) * morphEnd2Start;

	int level = int(abs(inLevel) - 1);
	float morphFactor;

	if (level > 0)
	{
		float morphRange = 0.25f;
		float distance = length(inTriangleVertex + inTriangleEdge0 * inBarycentricCoord.x + inTriangleEdge1 * inBarycentricCoord.y);
		float currLevelDistance = planetData[indirectIndex].PlanetLODDistanceLUT[level];
		float prevLevelDistance = planetData[indirectIndex].PlanetLODDistanceLUT[level - 1];
		morphFactor = 1.0f - (distance - currLevelDistance) / (prevLevelDistance - currLevelDistance);
		morphFactor = 1.0f - min(morphFactor, morphRange) / morphRange;
		morphFactor = clamp(morphFactor, 0, 1);
	}
	else
		morphFactor = 0;

	outSomething.x = morphFactor;

	// Acquire mixed barycentric position by interpolate from morphing start position and end position
	vec2 mixBarycentric = mix(inBarycentricCoord.xy, morphStart, morphFactor);

	// Acquire actual position with berycentric coordinate
	vec3 position = inTriangleVertex + inTriangleEdge0 * mixBarycentric.x + inTriangleEdge1 * mixBarycentric.y;



	vec3 planetPosition = position + perFrameData.wsCameraPosition.xyz;

	vec3 normal = normalize(planetPosition);

	float distToCamera = length(position);
	// FIXME: Remove this when I have a per-planet uniform containing planet related data including planet radius
	float radius = length(inTriangleVertex + perFrameData.wsCameraPosition.xyz);

	// Add a bias to adjust the factor
	float factor = distToCamera / (radius * globalData.PlanetRenderingSettings0.x);

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
