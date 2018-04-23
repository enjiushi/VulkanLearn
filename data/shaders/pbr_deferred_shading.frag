#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D GBuffer0[3];
layout (set = 3, binding = 3) uniform sampler2D GBuffer1[3];
layout (set = 3, binding = 4) uniform sampler2D GBuffer2[3];
layout (set = 3, binding = 5) uniform sampler2D DepthStencilBuffer[3];
layout (set = 3, binding = 6) uniform sampler2D ShadowMapDepthBuffer[3];
layout (set = 3, binding = 7) uniform sampler2D VerticalBlurredSSAOBuffer[3];

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inViewRay;

layout (location = 0) out vec4 outFragColor0;
layout (location = 1) out vec4 outFragColor1;

struct GBufferVariables
{
	vec4 albedo_roughness;
	vec4 normal_ao;
	vec4 world_position;
	float metalic;
	float shadowFactor;
	float ssaoFactor;
};

int index = int(perFrameData.camDir.a);

float AcquireShadowFactor(vec4 world_position)
{
	vec4 light_space_pos = globalData.mainLightVPN * world_position;
	light_space_pos /= light_space_pos.w;
	light_space_pos.xy = light_space_pos.xy * 0.5f + 0.5f;	// NOTE: Don't do this to z, as it's already within [0, 1] after vulkan ndc transform

	vec2 texelSize = 1.0f / textureSize(ShadowMapDepthBuffer[index], 0);
	float shadowFactor = 0.0f;
	float pcfDepth;

	pcfDepth = texture(ShadowMapDepthBuffer[index], light_space_pos.xy + vec2(-1, -1) * texelSize).r; 
	shadowFactor += (light_space_pos.z > pcfDepth ? 1.0 : 0.0) * 0.077847;

	pcfDepth = texture(ShadowMapDepthBuffer[index], light_space_pos.xy + vec2(0, -1) * texelSize).r; 
	shadowFactor += (light_space_pos.z > pcfDepth ? 1.0 : 0.0) * 0.123317;

	pcfDepth = texture(ShadowMapDepthBuffer[index], light_space_pos.xy + vec2(1, -1) * texelSize).r; 
	shadowFactor += (light_space_pos.z > pcfDepth ? 1.0 : 0.0) * 0.077847;

	pcfDepth = texture(ShadowMapDepthBuffer[index], light_space_pos.xy + vec2(-1, 0) * texelSize).r; 
	shadowFactor += (light_space_pos.z > pcfDepth ? 1.0 : 0.0) * 0.123317;

	pcfDepth = texture(ShadowMapDepthBuffer[index], light_space_pos.xy + vec2(0, 0) * texelSize).r; 
	shadowFactor += (light_space_pos.z > pcfDepth ? 1.0 : 0.0) * 0.195346;

	pcfDepth = texture(ShadowMapDepthBuffer[index], light_space_pos.xy + vec2(1, 0) * texelSize).r; 
	shadowFactor += (light_space_pos.z > pcfDepth ? 1.0 : 0.0) * 0.123317;

	pcfDepth = texture(ShadowMapDepthBuffer[index], light_space_pos.xy + vec2(-1, 1) * texelSize).r; 
	shadowFactor += (light_space_pos.z > pcfDepth ? 1.0 : 0.0) * 0.077847;

	pcfDepth = texture(ShadowMapDepthBuffer[index], light_space_pos.xy + vec2(0, 1) * texelSize).r; 
	shadowFactor += (light_space_pos.z > pcfDepth ? 1.0 : 0.0) * 0.123317;

	pcfDepth = texture(ShadowMapDepthBuffer[index], light_space_pos.xy + vec2(1, 1) * texelSize).r; 
	shadowFactor += (light_space_pos.z > pcfDepth ? 1.0 : 0.0) * 0.077847; 

	return 1.0f - shadowFactor;
}

GBufferVariables UnpackGBuffers(ivec2 coord, vec2 texcoord)
{
	GBufferVariables vars;

	vec4 gbuffer0 = texelFetch(GBuffer0[index], coord, 0);
	vec4 gbuffer1 = texelFetch(GBuffer1[index], coord, 0);
	vec4 gbuffer2 = texelFetch(GBuffer2[index], coord, 0);

	vars.albedo_roughness.rgb = gbuffer1.rgb;
	vars.albedo_roughness.a = gbuffer2.r;

	vars.normal_ao.xyz = normalize(gbuffer0.xyz * 2.0f - 1.0f);
	vars.normal_ao.w = gbuffer2.a;

	float linearDepth;
	vars.world_position = vec4(ReconstructPosition(coord, inViewRay, DepthStencilBuffer[index], linearDepth), 1.0);

	vars.metalic = gbuffer2.g;

	vars.shadowFactor = AcquireShadowFactor(vars.world_position);

	vars.ssaoFactor = AcquireBlurredSSAO(VerticalBlurredSSAOBuffer[index], texcoord, 1).r;
	vars.ssaoFactor = min(1.0f, vars.ssaoFactor * 2.0f);

	return vars;
}

void main() 
{
	ivec2 coord = ivec2(floor(inUv * globalData.gameWindowSize.xy));

	GBufferVariables vars = UnpackGBuffers(coord, inUv);

	vec3 n = vars.normal_ao.xyz;
	vec3 v = normalize(perFrameData.camPos.xyz - vars.world_position.xyz);
	vec3 l = globalData.mainLightDir.xyz;
	vec3 h = normalize(l + v);

	float NdotH = max(0.0f, dot(n, h));
	float NdotL = max(0.0f, dot(n, l));
	float NdotV = max(0.0f, dot(n, v));
	float LdotH = max(0.0f, dot(l, h));

	F0 = mix(F0, vars.albedo_roughness.rgb, vars.metalic);

	vec3 fresnel = Fresnel_Schlick(F0, LdotH);
	vec3 kD = (1.0 - vars.metalic) * (vec3(1.0) - fresnel);

	//-------------- Ambient -----------------------
	vec3 fresnel_roughness = Fresnel_Schlick_Roughness(F0, NdotV, vars.albedo_roughness.a);
	vec3 kD_roughness = (1.0 - vars.metalic) * (vec3(1.0) - fresnel_roughness);

	vec3 irradiance = texture(RGBA16_512_CUBE_IRRADIANCE, vec3(n.x, -n.y, n.z)).rgb * vars.albedo_roughness.rgb / PI;

	vec3 reflectSampleDir = reflect(-v, n);
	reflectSampleDir.y *= -1.0;

	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = vars.albedo_roughness.a * MAX_REFLECTION_LOD;
	vec3 reflect = textureLod(RGBA16_512_CUBE_PREFILTERENV, reflectSampleDir, lod).rgb;
	vec2 brdf_lut = texture(RGBA16_512_2D_BRDFLUT, vec2(NdotV, vars.albedo_roughness.a)).rg;

	// Here we use NdotV rather than LdotH, since L's direction is based on punctual light, and here ambient reflection calculation
	// requires reflection vector dot with N, which is RdotN, equals NdotV
	vec3 ambient = (reflect * (brdf_lut.x * fresnel_roughness + brdf_lut.y) + irradiance * kD_roughness) * min(vars.normal_ao.a, 1.0f - vars.ssaoFactor);
	//----------------------------------------------

	vec3 dirLightSpecular = fresnel * G_SchlicksmithGGX(NdotL, NdotV, vars.albedo_roughness.a) * GGX_D(NdotH, vars.albedo_roughness.a);
	vec3 dirLightDiffuse = vars.albedo_roughness.rgb * kD / PI;
	vec3 punctualRadiance = vars.shadowFactor * ((dirLightSpecular + dirLightDiffuse) * NdotL * globalData.mainLightColor.rgb);
	vec3 final = punctualRadiance + ambient;

	final = Uncharted2Tonemap(final * globalData.GEW.y);
	final = final * (1.0 / Uncharted2Tonemap(vec3(globalData.GEW.z)));
	final = pow(final, vec3(globalData.GEW.x));

	outFragColor1 = vec4(final, 1.0);
	outFragColor0 = vec4(punctualRadiance, 1.0);
}