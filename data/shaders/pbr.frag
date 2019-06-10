#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

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

#include "uniform_layout.sh"

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inWorldPos;
layout (location = 3) in vec3 inLightDir;
layout (location = 4) in vec3 inViewDir;
layout (location = 5) in vec3 inTangent;
layout (location = 6) in vec3 inBitangent;
layout (location = 7) flat in int perMaterialIndex;

layout (location = 0) out vec4 outFragColor;

vec3 perturbNormal(vec3 texNormal)
{
	vec3 tangentNormal = texNormal * 2.0 - 1.0;

	vec3 q1 = dFdx(inWorldPos);
	vec3 q2 = dFdy(inWorldPos);
	vec2 st1 = dFdx(inUv);
	vec2 st2 = dFdy(inUv);

	vec3 N = normalize(inNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

void main() 
{
	vec4 normal_ao;
	vec3 pertNormal;
	if (textures[perMaterialIndex].normalAOIndex < 0)
	{
		normal_ao = vec4(normalize(inNormal), textures[perMaterialIndex].AOMetalic.r);
		pertNormal = normal_ao.xyz;
	}
	else
	{
		normal_ao = texture(RGBA8_1024_MIP_2DARRAY, vec3(inUv.st, textures[perMaterialIndex].normalAOIndex), 0.0);
		pertNormal = perturbNormal(normal_ao.xyz);

		//pertNormal = normal_ao.xyz * 2.0 - 1.0;
		//mat3 TBN = mat3(normalize(inTangent), normalize(inBitangent), normalize(inNormal));
		//pertNormal = TBN * pertNormal;
		//pertNormal = normalize(inNormal);
	}

	vec3 n = pertNormal;
	//vec3 n = normalize(inNormal);
	vec3 v = normalize(inViewDir);
	vec3 l = normalize(inLightDir);
	vec3 h = normalize(l + v);

	float NdotH = max(0.0f, dot(n, h));
	float NdotL = max(0.0f, dot(n, l));
	float NdotV = max(0.0f, dot(n, v));
	float LdotH = max(0.0f, dot(l, h));

	vec4 albedo_roughness;
	if (textures[perMaterialIndex].albedoRoughnessIndex < 0)
		albedo_roughness = vec4(vec3(1.0), 1.0);
	else
		albedo_roughness = texture(RGBA8_1024_MIP_2DARRAY, vec3(inUv.st, textures[perMaterialIndex].albedoRoughnessIndex), 0.0);

	albedo_roughness *= textures[perMaterialIndex].albedoRougness;
	albedo_roughness.rgb = pow(albedo_roughness.rgb, vec3(2.2));

	float metalic;
	if (textures[perMaterialIndex].metallicIndex < 0)
		metalic = 1.0;
	else
		metalic = texture(R8_1024_MIP_2DARRAY, vec3(inUv.st, textures[perMaterialIndex].metallicIndex), 0.0).r;

	metalic *= textures[perMaterialIndex].AOMetalic.g;

	F0 = mix(F0, albedo_roughness.rgb, metalic);

	vec3 fresnel = Fresnel_Schlick(F0, LdotH);
	vec3 kD = (1.0 - metalic) * (vec3(1.0) - fresnel);

	//-------------- Ambient -----------------------
	vec3 fresnel_roughness = Fresnel_Schlick_Roughness(F0, NdotV, albedo_roughness.a);
	vec3 kD_roughness = (1.0 - metalic) * (vec3(1.0) - fresnel_roughness);

	vec3 irradiance = texture(RGBA16_512_CUBE_IRRADIANCE, vec3(n.x, -n.y, n.z)).rgb * albedo_roughness.rgb / PI;

	vec3 reflectSampleDir = reflect(-v, n);
	reflectSampleDir.y *= -1.0;

	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = albedo_roughness.a * MAX_REFLECTION_LOD;
	vec3 reflect = textureLod(RGBA16_512_CUBE_PREFILTERENV, reflectSampleDir, lod).rgb;
	vec2 brdf_lut = texture(RGBA16_512_2D_BRDFLUT, vec2(NdotV, albedo_roughness.a)).rg;

	// Here we use NdotV rather than LdotH, since L's direction is based on punctual light, and here ambient reflection calculation
	// requires reflection vector dot with N, which is RdotN, equals NdotV
	vec3 ambient = (reflect * (brdf_lut.x * fresnel_roughness + brdf_lut.y) + irradiance * kD_roughness) * normal_ao.a;
	//----------------------------------------------

	vec3 dirLightSpecular = fresnel * G_SchlicksmithGGX(NdotL, NdotV, albedo_roughness.a) * GGX_D(NdotH, albedo_roughness.a);
	vec3 dirLightDiffuse = albedo_roughness.rgb * kD / PI;
	vec3 final = normal_ao.a * ((dirLightSpecular + dirLightDiffuse) * NdotL * globalData.mainLightColor.rgb) + ambient;

	final = Uncharted2Tonemap(final * globalData.GEW.y);
	final = final * (1.0 / Uncharted2Tonemap(vec3(globalData.GEW.z)));
	final = pow(final, vec3(globalData.GEW.x));

	outFragColor = vec4(final, 1.0);
}