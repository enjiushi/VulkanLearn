#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 3, binding = 1) uniform sampler2D albedoTex;
layout (set = 3, binding = 2) uniform sampler2D bumpTex;
layout (set = 3, binding = 3) uniform sampler2D metalicTex;
layout (set = 3, binding = 4) uniform samplerCube irradianceTex;
layout (set = 3, binding = 5) uniform samplerCube prefilterEnvTex;
layout (set = 3, binding = 6) uniform sampler2D BRDFLut;

#include "uniform_layout.h"

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inWorldPos;
layout (location = 3) in vec3 inLightDir;
layout (location = 4) in vec3 inViewDir;
layout (location = 5) in vec3 inTangent;
layout (location = 6) in vec3 inBitangent;

layout (location = 0) out vec4 outFragColor;

vec3 perturbNormal()
{
	vec3 tangentNormal = texture(bumpTex, inUv.st).xyz * 2.0 - 1.0;

	vec3 q1 = dFdx(inWorldPos);
	vec3 q2 = dFdy(inWorldPos);
	vec2 st1 = dFdx(inUv.st);
	vec2 st2 = dFdy(inUv.st);

	vec3 N = normalize(inNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

void main() 
{
	vec4 normal_ao = texture(bumpTex, inUv.st, 0.0);
	vec3 pertNormal = normal_ao.xyz * 2.0 - vec3(1.0);

	mat3 TBN = mat3(inTangent, inBitangent, inNormal);
	pertNormal = TBN * pertNormal;

	vec3 n = normalize(pertNormal);
	//vec3 n = normalize(inNormal);
	vec3 v = normalize(inViewDir);
	vec3 l = normalize(inLightDir);
	vec3 h = normalize(l + v);

	float NdotH = max(0.0f, dot(n, h));
	float NdotL = max(0.0f, dot(n, l));
	float NdotV = max(0.0f, dot(n, v));
	float LdotH = max(0.0f, dot(l, h));

	vec4 albedo_roughness = texture(albedoTex, inUv.st, 0.0);
	albedo_roughness.rgb = pow(albedo_roughness.rgb, vec3(2.2));

	//vec3 albedo = pow(texture(albedoTex, inUv.st, 0.0).rgb, vec3(2.2));
	//float roughness = texture(roughnessTex, inUv.st, 0.0).r;

	vec3 normal = texture(bumpTex, inUv.st, 0.0).rgb;
	float metalic = texture(metalicTex, inUv.st, 0.0).r;
	F0 = mix(F0, albedo_roughness.rgb, metalic);

	vec3 fresnel = Fresnel_Schlick(F0, LdotH);
	vec3 kD = (1.0 - metalic) * (vec3(1.0) - fresnel);

	//-------------- Ambient -----------------------
	vec3 fresnel_roughness = Fresnel_Schlick_Roughness(F0, NdotV, albedo_roughness.a);
	vec3 kD_roughness = (1.0 - metalic) * (vec3(1.0) - fresnel_roughness);

	vec3 irradiance = texture(irradianceTex, vec3(n.x, -n.y, n.z)).rgb * albedo_roughness.rgb / PI;

	//vec3 reflectSampleDir = NdotV * n * 2.0 - v;
	//reflectSampleDir.y *= -1.0;
	vec3 reflectSampleDir = reflect(-v, n);
	reflectSampleDir.y *= -1.0;

	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = albedo_roughness.a * MAX_REFLECTION_LOD;
	vec3 reflect = textureLod(prefilterEnvTex, reflectSampleDir, lod).rgb;
	vec2 brdf_lut = texture(BRDFLut, vec2(NdotV, albedo_roughness.a)).rg;

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