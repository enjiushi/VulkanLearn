#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) flat in int perMaterialIndex;
layout (location = 5) flat in int perObjectIndex;
layout (location = 6) in vec3 inWorldPos;
layout (location = 7) in vec3 inEyePos;
layout (location = 8) noperspective in vec2 inScreenPos;
layout (location = 9) in vec3 inPrevWorldPos;
layout (location = 10) in vec3 inDistToEdge;

layout (location = 0) out vec4 outGBuffer0;
layout (location = 1) out vec4 outGBuffer1;
layout (location = 2) out vec4 outGBuffer2;
layout (location = 3) out vec4 outMotionVec;

#include "uniform_layout.sh"

struct PBRTextures
{
	vec4 albedoRougness;
	vec2 AOMetalic;

	float albedoRoughnessIndex;
	float normalAOIndex;
	float metallicIndex;
};

layout(set = 3, binding = 0) buffer MaterialUniforms
{
	PBRTextures textures[];
};

void main() 
{
	float metalic = 0.9f;

	vec4 normal_ao = vec4(vec3(0), 1);
	normal_ao.xyz = normalize(inNormal);

	vec4 albedo_roughness = vec4(vec3(1), 0.2f);
	vec3 edgeFactor = 1.0f - inDistToEdge;
	edgeFactor = step(vec3(0.99f), edgeFactor);
	float edge = max(max(edgeFactor.x, edgeFactor.y), edgeFactor.z);
	metalic = mix(metalic, 0, edge);
	albedo_roughness.w = mix(albedo_roughness.w, 1, edge);

	outGBuffer0.xyz = normal_ao.xyz * 0.5f + 0.5f;
	outGBuffer0.w = albedo_roughness.w;

	outGBuffer1 = vec4(albedo_roughness.rgb, 0);
	outGBuffer2 = vec4(vec3(albedo_roughness.w, metalic, 0), normal_ao.a);

	vec4 prevNDCPos = perFrameData.prevVP * vec4(inPrevWorldPos, 1.0f);
	vec2 prevTexCoord = (prevNDCPos.xy / prevNDCPos.w);

	outMotionVec.rg = prevTexCoord - inScreenPos;
	outMotionVec.rg = outMotionVec.rg * 0.5f;

	// Calculate circle of confusion
	outGBuffer1.a = CalculateCoC(-inEyePos.z);
}
