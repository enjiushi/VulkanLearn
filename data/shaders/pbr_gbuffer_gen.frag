#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) flat in int perMaterialIndex;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outGBuffer0;
layout (location = 2) out vec4 outGBuffer1;
layout (location = 3) out vec4 outGBuffer2;

#include "uniform_layout.h"

struct PBRTextures
{
	vec4 albedoRougness;
	vec2 AOMetalic;

	float albedoRoughnessIndex;
	float normalAOIndex;
	float metallicIndex;
};

layout(set = 3, binding = 1) buffer MaterialUniforms
{
	PBRTextures textures[];
};

void main() 
{
	outColor = vec4(1);

	vec4 normal_ao = vec4(vec3(0), 1);
	if (textures[perMaterialIndex].normalAOIndex < 0)
	{
		normal_ao.xyz = normalize(inTangent);
		normal_ao.w = textures[perMaterialIndex].AOMetalic.x;
	}
	else
	{
		normal_ao = texture(RGBA8_1024_MIP_2DARRAY, vec3(inUv.st, textures[perMaterialIndex].normalAOIndex), 0.0);

		vec3 pertNormal = normal_ao.xyz * 2.0 - 1.0;
		mat3 TBN = mat3(normalize(inTangent), normalize(inBitangent), normalize(inNormal));
		pertNormal = TBN * pertNormal;

		normal_ao.xy = pertNormal.xy;
	}

	outColor.xyz = normal_ao.xyz;

	outGBuffer0.xy = normal_ao.xy;
	outGBuffer0.z = normal_ao.w;
	outGBuffer0.w = 0;

	outGBuffer1 = vec4(0);
	outGBuffer2 = vec4(0);
}
