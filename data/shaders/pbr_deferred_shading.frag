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
layout (set = 3, binding = 9) uniform sampler2D SSRInfo[3];
layout (set = 3, binding = 10) uniform sampler2D TemporalResult[2];

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec2 inOneNearPosition;

layout (location = 0) out vec4 outShadingColor;

int index = int(perFrameData.camDir.a);
int pingpong = int(perFrameData.camPos.a);

const vec2 offset[4] =
{
	vec2(0, 0),
	vec2(2, -2),
	vec2(-2, -2),
	vec2(0, 2)
};


vec4 CalculateSSR(vec3 n, vec3 v, float NdotV, vec4 albedoRoughness, vec3 wsPosition, float metalic)
{
	ivec2 coord = ivec2(inUv * globalData.gameWindowSize.xy);

	vec4 SSRRadiance = vec4(0);
	float weightSum = 0.0f;

	vec2 rand2 = normalize(PDsrand2(inUv + vec2(perFrameData.time.x)));
	mat2 offsetRotation = mat2(rand2.x, rand2.y, -rand2.y, rand2.x);

	int count = 4;
	for (int i = 0; i < count; i++)
	{
		vec4 SSRHitInfo = texelFetch(SSRInfo[index], (coord + ivec2(offsetRotation * offset[i])) / 2, 0);
		float hitFlag = sign(SSRHitInfo.a) * 0.5f + 0.5f;
		vec3 SSRSurfColor = (texelFetch(TemporalResult[pingpong], ivec2(SSRHitInfo.xy), 0).rgb) * hitFlag;

		float SSRSurfDepth;
		vec3 SSRSurfPosition = ReconstructWSPosition(ivec2(SSRHitInfo.xy), DepthStencilBuffer[index], SSRSurfDepth);

		vec3 l = normalize(SSRSurfPosition.xyz - wsPosition);
		vec3 h = normalize(l + v);

		float NdotH = max(0.0f, dot(n, h));
		float NdotL = max(0.0f, dot(n, l));
		float LdotH = max(0.0f, dot(l, h));

		float weight;
		if (hitFlag > 0.5f)
			weight = GGX_D(NdotH, albedoRoughness.a) * G_SchlicksmithGGX(NdotL, NdotV, albedoRoughness.a) / (4.0f * NdotL * NdotV + 0.001f) / max(SSRHitInfo.b, 1e-5);
		else
			weight = 0.5f;	// FIXME?
		weight = max(weight, 0.001f);

		SSRRadiance.rgb += SSRSurfColor * weight;
		SSRRadiance.a += hitFlag * weight;

		weightSum += weight;
	}

	SSRRadiance /= weightSum;	// normalize
	
	// Pre-integrated FG
	//F0 = mix(F0, vars.albedo_roughness.rgb, vars.metalic);
	//vec2 brdf_lut = texture(RGBA16_512_2D_BRDFLUT, vec2(NdotV, vars.albedo_roughness.a)).rg;
	//vec3 fresnel_roughness = Fresnel_Schlick_Roughness(F0, NdotV, vars.albedo_roughness.a);

	//SSRRadiance.rgb = SSRRadiance.rgb * (brdf_lut.x * fresnel_roughness + brdf_lut.y);

	return SSRRadiance;
}

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

	vec2 brdf_lut = texture(RGBA16_512_2D_BRDFLUT, vec2(NdotV, vars.albedo_roughness.a)).rg;

	// Here we use NdotV rather than LdotH, since L's direction is based on punctual light, and here ambient reflection calculation
	// requires reflection vector dot with N, which is RdotN, equals NdotV
	vec4 SSRRadiance = CalculateSSR(n, v, NdotV, vars.albedo_roughness, vars.world_position.xyz, vars.metalic);
	vec4 radiance = vec4(mix(reflect, SSRRadiance.rgb, SSRRadiance.a), min(vars.normal_ao.a, 1.0f - vars.ssaoFactor));
	radiance.rgb *= (brdf_lut.x * fresnel_roughness + brdf_lut.y);
	vec3 ambient = (irradiance * kD_roughness + radiance.rgb) * radiance.a;
	//vec3 ambient = radiance.rgb;
	//----------------------------------------------

	vec3 dirLightSpecular = fresnel * G_SchlicksmithGGX(NdotL, NdotV, vars.albedo_roughness.a) * min(1.0f, GGX_D(NdotH, vars.albedo_roughness.a)) / (4.0f * NdotL * NdotV + 0.001f);
	vec3 dirLightDiffuse = vars.albedo_roughness.rgb * kD / PI;
	vec3 punctualRadiance = vars.shadowFactor * ((dirLightSpecular + dirLightDiffuse) * NdotL * globalData.mainLightColor.rgb);
	outShadingColor = vec4(punctualRadiance + ambient, 1.0f);



	//vec2 motionVec = texture(MotionVector[index], inUv).rg;

	//vec4 SSRRadiance = CalculateSSR(n, v, NdotV, albedo, F0, fresnel_roughness, brdf_lut);
}