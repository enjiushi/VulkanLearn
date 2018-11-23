#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D GBuffer0[3];
layout (set = 3, binding = 3) uniform sampler2D GBuffer1[3];
layout (set = 3, binding = 4) uniform sampler2D GBuffer2[3];
layout (set = 3, binding = 5) uniform sampler2D MotionVector[3];
layout (set = 3, binding = 6) uniform sampler2D DepthStencilBuffer[3];
layout (set = 3, binding = 7) uniform sampler2D ShadowMapDepthBuffer[3];
layout (set = 3, binding = 8) uniform sampler2D BlurredSSAOBuffer[3];

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec2 inOneNearPosition;

layout (location = 0) out vec4 outShadingColor;
layout (location = 1) out vec4 outEnvReflColor;

int index = int(perFrameData.camDir.a);

void main() 
{
	ivec2 coord = ivec2(floor(inUv * globalData.gameWindowSize.xy));

	GBufferVariables vars = UnpackGBuffers(coord, inUv, inOneNearPosition, GBuffer0[index], GBuffer1[index], GBuffer2[index], DepthStencilBuffer[index], BlurredSSAOBuffer[index], ShadowMapDepthBuffer[index]);

	if (length(vars.normal_ao.xyz) > 1.1f)
		discard;

	vec3 n = normalize(vars.normal_ao.xyz);
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
	//vec4 reflectSSRInfo = texture(SSRInfo[index], inUv);
	//reflect = mix(reflect, vec3(0, 0, 0), reflectSSRInfo.w);outEnvReflColor

	vec2 brdf_lut = texture(RGBA16_512_2D_BRDFLUT, vec2(NdotV, vars.albedo_roughness.a)).rg;

	// Here we use NdotV rather than LdotH, since L's direction is based on punctual light, and here ambient reflection calculation
	// requires reflection vector dot with N, which is RdotN, equals NdotV
	outEnvReflColor.rgb = reflect * (brdf_lut.x * fresnel_roughness + brdf_lut.y);
	outEnvReflColor.a = min(vars.normal_ao.a, 1.0f - vars.ssaoFactor);
	vec3 ambient = (irradiance * kD_roughness) * outEnvReflColor.a;
	//----------------------------------------------

	vec3 dirLightSpecular = fresnel * G_SchlicksmithGGX(NdotL, NdotV, vars.albedo_roughness.a) * min(1.0f, GGX_D(NdotH, vars.albedo_roughness.a)) / (4.0f * NdotL * NdotV + 0.001f);
	vec3 dirLightDiffuse = vars.albedo_roughness.rgb * kD / PI;
	vec3 punctualRadiance = vars.shadowFactor * ((dirLightSpecular + dirLightDiffuse) * NdotL * globalData.mainLightColor.rgb);
	outShadingColor = vec4(punctualRadiance + ambient, 1.0f);
}