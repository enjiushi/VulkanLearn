#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D GBuffer0[3];
layout (set = 3, binding = 3) uniform sampler2D GBuffer1[3];
layout (set = 3, binding = 4) uniform sampler2D GBuffer2[3];
layout (set = 3, binding = 5) uniform sampler2D MotionVector[3];
layout (set = 3, binding = 6) uniform sampler2D DepthStencilBuffer[3];
layout (set = 3, binding = 7) uniform sampler2D ShadingResult[3];
layout (set = 3, binding = 8) uniform sampler2D EnvReflResult[3];
layout (set = 3, binding = 9) uniform sampler2D SSRInfo[3];
layout (set = 3, binding = 10) uniform sampler2D TemporalShading;
layout (set = 3, binding = 11) uniform sampler2D TemporalSSR;

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec2 inOneNearPosition;

layout (location = 0) out vec4 outTemporalShading;
layout (location = 1) out vec4 outTemporalSSR;
layout (location = 2) out vec4 outFinal;

int index = int(perFrameData.camDir.a);

const vec2 offset[4] =
{
	vec2(0, 0),
	vec2(2, -2),
	vec2(-2, -2),
	vec2(0, 2)
};

vec4 CalculateSSR(vec2 texcoord)
{
	ivec2 coord = ivec2(texcoord * globalData.gameWindowSize.xy);
	GBufferVariables vars = UnpackGBuffers(coord, inOneNearPosition, GBuffer0[index], GBuffer1[index], GBuffer2[index], DepthStencilBuffer[index]);

	vec3 n = vars.normal_ao.xyz;
	vec3 v = normalize(perFrameData.camPos.xyz - vars.world_position.xyz);

	float NdotV = max(0.0f, dot(n, v));

	vec3 albedo = mix(F0, vars.albedo_roughness.rgb, vars.metalic);

	vec4 SSRRadiance = vec4(0);
	float weightSum = 0.0f;

	vec2 rand2 = normalize(PDsrand2(inUv + vec2(perFrameData.time.x)));
	mat2 offsetRotation = mat2(rand2.x, rand2.y, -rand2.y, rand2.x);

	int count = 4;
	for (int i = 0; i < count; i++)
	{
		vec4 SSRInfo = texelFetch(SSRInfo[index], (coord + ivec2(offsetRotation * offset[i])) / 2, 0);
		float hitFlag = sign(SSRInfo.a) * 0.5f + 0.5f;
		vec3 SSRSurfColor = ((texelFetch(ShadingResult[index], ivec2(SSRInfo.xy), 0).rgb) + (texelFetch(EnvReflResult[index], ivec2(SSRInfo.xy), 0).rgb)) * hitFlag;

		float SSRSurfDepth;
		vec3 SSRSurfPosition = ReconstructWSPosition(coord, DepthStencilBuffer[index], SSRSurfDepth);

		vec3 l = normalize(SSRSurfPosition.xyz - vars.world_position.xyz);
		vec3 h = normalize(l + v);

		float NdotH = max(0.0f, dot(n, h));
		float NdotL = max(0.0f, dot(n, l));
		float LdotH = max(0.0f, dot(l, h));

		vec3 fresnel = Fresnel_Schlick(albedo, NdotL);
		vec3 kD = (1.0 - vars.metalic) * (vec3(1.0) - fresnel);

		float weight;
		if (hitFlag > 0.5f)
			weight = GGX_D(NdotH, vars.albedo_roughness.a) * G_SchlicksmithGGX(NdotL, NdotV, vars.albedo_roughness.a) / (4.0f * NdotL * NdotV + 0.001f) / max(SSRInfo.b, 1e-5);
		else
			weight = 0.5f;	// FIXME?
		weight = max(weight, 0.001f);

		SSRRadiance.rgb += SSRSurfColor * weight;
		SSRRadiance.a += hitFlag * weight;

		weightSum += weight;
	}

	SSRRadiance /= weightSum;	// normalize
	
	// Pre-integrated FG
	F0 = mix(F0, vars.albedo_roughness.rgb, vars.metalic);
	vec2 brdf_lut = texture(RGBA16_512_2D_BRDFLUT, vec2(NdotV, vars.albedo_roughness.a)).rg;
	vec3 fresnel_roughness = Fresnel_Schlick_Roughness(F0, NdotV, vars.albedo_roughness.a);

	SSRRadiance.rgb = SSRRadiance.rgb * (brdf_lut.x * fresnel_roughness + brdf_lut.y);

	return SSRRadiance;
}

vec3 resolve(sampler2D currSampler, sampler2D prevSampler, vec2 unjitteredUV, vec2 motionVec)
{
	vec3 curr = texture(currSampler, unjitteredUV).rgb;
	vec3 prev = texture(prevSampler, inUv + motionVec).rgb;

	vec2 u = vec2(globalData.gameWindowSize.z, 0);
	vec2 v = vec2(0, globalData.gameWindowSize.w);

	vec3 bl = texture(ShadingResult[index], unjitteredUV - u - v).rgb;
	vec3 bm = texture(ShadingResult[index], unjitteredUV - v).rgb;
	vec3 br = texture(ShadingResult[index], unjitteredUV + u - v).rgb;
	vec3 ml = texture(ShadingResult[index], unjitteredUV - u).rgb;
	vec3 mr = texture(ShadingResult[index], unjitteredUV + u).rgb;
	vec3 tl = texture(ShadingResult[index], unjitteredUV - u + v).rgb;
	vec3 tm = texture(ShadingResult[index], unjitteredUV + v).rgb;
	vec3 tr = texture(ShadingResult[index], unjitteredUV + u + v).rgb;

	vec3 minColor = min(bl, min(bm, min(br, min(ml, min(mr, min(tl, min(tm, min(tr, curr))))))));
	vec3 maxColor = max(bl, max(bm, max(br, max(ml, max(mr, max(tl, max(tm, max(tr, curr))))))));

	// min max suppress to reduce temporal ghosting effect
	vec3 clippedPrev = clip_aabb(minColor, maxColor, prev);
	prev = mix(clippedPrev, prev, 0.8f);

	float currLum = Luminance(curr);
	float prevLum = Luminance(prev);

	float unbiasedDiff = abs(currLum - prevLum) / max(currLum, max(prevLum, 0.2f));
	float unbiasedWeight = 1.0 - unbiasedDiff;
	float unbiasedWeightSQR = unbiasedWeight * unbiasedWeight;
	float feedback = mix(0.87f, 0.97f, unbiasedWeightSQR);

	return mix(curr, prev, 0.97f);
}

void main() 
{
	vec2 unjitteredUV = inUv - perFrameData.cameraJitterOffset;
	
	vec2 motionVec = texture(MotionVector[index], unjitteredUV).rg;

	vec4 SSRRadiance = CalculateSSR(unjitteredUV);
	vec4 envReflect = texture(EnvReflResult[index], unjitteredUV).rgba;
	vec4 prevSSR = texture(TemporalSSR, inUv + motionVec).rgba;

	// 1. SSR, EnvReflection resolve
	// 2. SSR temporal filter
	outTemporalSSR = mix(vec4(mix(envReflect.rgb, SSRRadiance.rgb, SSRRadiance.a), SSRRadiance.a), prevSSR, 0.98f);

	vec3 result = resolve(ShadingResult[index], TemporalShading, unjitteredUV, motionVec);

	outTemporalShading = vec4(result, 1.0f);

	result += outTemporalSSR.rgb;

	outFinal = vec4(result, 1.0);
}