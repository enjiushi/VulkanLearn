#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inCSNormal;
layout (location = 2) in vec3 inCSPosition;
layout (location = 3) noperspective in vec2 inScreenPosition;
layout (location = 4) in vec3 inPrevCSPosition;
layout (location = 5) in vec4 inBarycentricCoord;

layout (location = 0) out vec4 outGBuffer0;
layout (location = 1) out vec4 outGBuffer1;
layout (location = 2) out vec4 outGBuffer2;
layout (location = 3) out vec4 outMotionVec;

#include "uniform_layout.sh"
#include "global_parameters.sh"
#include "utilities.sh"

void main() 
{
	float metalic = 0.9f;

	vec4 normalAO = vec4(vec3(0), 1);
	normalAO.xyz = normalize(inCSNormal);

	float patchEdge = min(min(inBarycentricCoord.z, inBarycentricCoord.w), 1.0f - inBarycentricCoord.z - inBarycentricCoord.w);
	patchEdge = step(1.0f - globalData.PlanetRenderingSettings1.y, 1.0f - patchEdge) * globalData.PlanetRenderingSettings1.x;

	// NOTE: Cut subdivide count half since we're rendering 4 sub-tiles to form a whole tile
	float patchSubdivideCount = globalData.PlanetRenderingSettings0.w * 0.5f;
	float edge0 = inBarycentricCoord.x * patchSubdivideCount - floor(inBarycentricCoord.x * patchSubdivideCount);
	float edge1 = inBarycentricCoord.y * patchSubdivideCount  - floor(inBarycentricCoord.y * patchSubdivideCount);
	float edge2 = ceil((inBarycentricCoord.x + inBarycentricCoord.y) * patchSubdivideCount) - (inBarycentricCoord.x + inBarycentricCoord.y) * patchSubdivideCount ;
	float trianlgeEdge = min(min(edge0, edge1), edge2);
	trianlgeEdge = step(1.0f - globalData.PlanetRenderingSettings1.z, 1.0f - trianlgeEdge) * globalData.PlanetRenderingSettings1.x;

	vec4 albedoRoughness = vec4(mix(vec3(1), vec3(1, 0, 0), patchEdge), 0.2f);
	metalic = mix(metalic, 0, trianlgeEdge);
	albedoRoughness.w = mix(albedoRoughness.w, 1, trianlgeEdge);

	outGBuffer0.xyz = normalAO.xyz * 0.5f + 0.5f;
	outGBuffer0.w = albedoRoughness.w;

	outGBuffer1 = vec4(albedoRoughness.rgb, 0);
	outGBuffer2 = vec4(vec3(albedoRoughness.w, metalic, 0), normalAO.a);

	vec4 prevNDCPos = globalData.projection * vec4(inPrevCSPosition, 1.0f);
	vec2 prevTexCoord = (prevNDCPos.xy / prevNDCPos.w);

	outMotionVec.rg = prevTexCoord - inScreenPosition;
	outMotionVec.rg = outMotionVec.rg * 0.5f;

	// Calculate circle of confusion
	outGBuffer1.a = CalculateCoC(-inCSPosition.z);
}
