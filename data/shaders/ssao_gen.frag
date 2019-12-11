#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

layout (set = 3, binding = 3) uniform sampler2D GBuffer0[3];
layout (set = 3, binding = 4) uniform sampler2D GBuffer2[3];
layout (set = 3, binding = 5) uniform sampler2D DepthStencilBuffer[3];

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec2 inOneNearPosition;
layout (location = 2) in vec3 inWsView;

layout (location = 0) out vec4 outSSAOFactor;
layout (location = 1) out vec4 outSSRInfo;

layout(push_constant) uniform PushConsts {
	layout (offset = 0) float blueNoiseTexIndex;
} pushConsts;

int index = int(perFrameData.camDir.a);

float maxRegenCount = globalData.SSRSettings0.z;
float surfaceMargin = globalData.SSRSettings0.w;
float rayTraceStride = globalData.SSRSettings1.x;
float rayTraceInitOffset = globalData.SSRSettings1.y;
float rayTraceMaxStep = globalData.SSRSettings1.z;
float rayTraceHitThickness = globalData.SSRSettings1.w;

void UnpackNormalRoughness(ivec2 coord, out vec3 normal, out float roughness)
{
	vec4 gbuffer0 = texelFetch(GBuffer0[index], coord, 0);
	vec4 gbuffer2 = texelFetch(GBuffer2[index], coord, 0);

	normal = length(gbuffer0) < 0.001f ? vec3(0.0f) : normalize(gbuffer0.xyz * 2.0f - 1.0f);
	roughness = gbuffer2.r;
}

vec4 RayMarch(vec3 sampleNormal, vec3 normal, vec3 position, vec3 wsViewRay)
{
	if (length(sampleNormal) < 0.5f)
		return vec4(0.0f);

	float maxDistance = 20.0f;
	float csNearPlane = -perFrameData.nearFarAB.x;

	vec3 wsReflectDir = reflect(wsViewRay, sampleNormal);

	if (dot(wsReflectDir, normal) < 0.0f)
		return vec4(0.0f);

	vec4 csReflectDir = perFrameData.view * vec4(wsReflectDir, 0.0f);
	vec4 csRayOrigin = perFrameData.view * vec4(position, 1.0f);

	float rayLength = (csRayOrigin.z + csReflectDir.z * maxDistance > csNearPlane) ? (csNearPlane - csRayOrigin.z) / csReflectDir.z : maxDistance;

	vec4 csRayEnd = csRayOrigin + csReflectDir * rayLength;

	vec4 clipRayOrigin = globalData.projection * csRayOrigin;
	vec4 clipRayEnd = globalData.projection * csRayEnd;

	float k0 = 1.0f / clipRayOrigin.w;
	float k1 = 1.0f / clipRayEnd.w;

	vec3 Q0 = csRayOrigin.xyz * k0;
	vec3 Q1 = csRayEnd.xyz * k1;

	vec2 P0 = clipRayOrigin.xy * k0 * 0.5f + 0.5f;
	vec2 P1 = clipRayEnd.xy * k1 * 0.5f + 0.5f;

	P0 *= globalData.gameWindowSize.xy;
	P1 *= globalData.gameWindowSize.xy;

	vec2 screenOffset = P1 - P0;
	float sqScreenDist = dot(screenOffset, screenOffset);
	P1 += step(sqScreenDist, 0.0001f) * vec2(0.01f);

	bool permute = false;
	if (abs(screenOffset.x) < abs(screenOffset.y))
	{
		permute = true;
		screenOffset = screenOffset.yx;
		P0 = P0.yx;
		P1 = P1.yx;
	}

	float stepDirection = sign(screenOffset.x);
	float stepInterval = stepDirection / screenOffset.x;

	vec3 dQ = (Q1 - Q0) * stepInterval * rayTraceStride;
	float dk = (k1 - k0) * stepInterval * rayTraceStride;
	vec2 dP = vec2(stepDirection, screenOffset.y * stepInterval) * rayTraceStride;

	float jitter = PDsrand(inUv + vec2(perFrameData.time.x));
	float init = rayTraceInitOffset + jitter;

	vec3 Q = Q0 + dQ * init;
	float k = k0 + dk * init;
	vec2 P = P0 + dP * init;

	float end = P1.x * stepDirection;

	float stepCount = 0.0f;

	float prevZMax = csRayOrigin.z;
	float ZMin = prevZMax;
	float ZMax = prevZMax;
	float sampleZ = prevZMax - 100000;

	vec2 hit;

	for (;((P.x * stepDirection) <= end) &&
			(stepCount <= rayTraceMaxStep - 1) &&
			//(ZMax > (sampleZ - rayTraceHitThickness)) &&
			((ZMax > sampleZ) || (ZMin < sampleZ - rayTraceHitThickness)) && 
			sampleZ != 0.0f;
			P += dP, Q.z += dQ.z, k += dk, stepCount++)
	{
		ZMin = prevZMax;
		ZMax = (Q.z + dQ.z * 0.5f) / (k + dk * 0.5f);
		prevZMax = ZMax;

		if (ZMin < ZMax)
		{
			float t = ZMin;
			ZMin = ZMax;
			ZMax = t;
		}

		hit = permute ? P.yx : P;

		float window_z = texelFetch(DepthStencilBuffer[index], ivec2(hit), 0).r;
		sampleZ = ReconstructLinearDepth(window_z);
	}

	vec4 rayHitInfo;

	rayHitInfo.rg = hit;

	vec3 hitNormal;
	float roughness;
	UnpackNormalRoughness(ivec2(hit), hitNormal, roughness);

	rayHitInfo.b = stepCount;
	rayHitInfo.a = float((ZMax < sampleZ) && (ZMin > sampleZ - rayTraceHitThickness) && (dot(hitNormal, wsReflectDir) < 0));


	return rayHitInfo;
}

void main() 
{
	ivec2 coord = ivec2(floor(inUv * globalData.gameWindowSize.xy));

	vec3 normal;
	float roughness;
	UnpackNormalRoughness(coord, normal, roughness);

	float linearDepth;
	vec3 position = ReconstructWSPosition(coord, inOneNearPosition, DepthStencilBuffer[index], linearDepth);

	vec3 tangent = texture(SSAO_RANDOM_ROTATIONS, inUv * globalData.SSAOWindowSize.xy / textureSize(SSAO_RANDOM_ROTATIONS, 0)).xyz * 2.0f - 1.0f;
	tangent = normalize(tangent - dot(normal, tangent) * normal);

	vec3 bitangent = normalize(cross(normal, tangent));

	mat3 TBN = mat3(tangent, bitangent, normal);

	// Calculate camera space width of this particular depth of current pixel
	float csWidth = globalData.MainCameraSettings3.x * abs(linearDepth) * 2;
	float ssaoRaiusInScreenSapce = globalData.SSAOSettings.y / csWidth;

	float occlusion = 0.0f;

	if (ssaoRaiusInScreenSapce > globalData.SSAOSettings.w)
	{
		float ssaoAvailableFactor = min(1.0f, (ssaoRaiusInScreenSapce - globalData.SSAOSettings.w) / (globalData.SSAOSettings.z - globalData.SSAOSettings.w));

		for (int i = 0; i < int(globalData.SSAOSettings.x); i++)
		{
			vec3 sampleDir = TBN * globalData.SSAOSamples[i].xyz;
			vec3 samplePos = position + sampleDir * globalData.SSAOSettings.y;

			vec4 clipSpaceSample = perFrameData.VP * vec4(samplePos, 1.0f);
			clipSpaceSample = clipSpaceSample / clipSpaceSample.w;
			clipSpaceSample.xy = clipSpaceSample.xy * 0.5f + 0.5f;

			float sampledDepth = clipSpaceSample.z;
			float textureDepth = texture(DepthStencilBuffer[index], clipSpaceSample.xy).r;

			sampledDepth = ReconstructLinearDepth(sampledDepth);
			textureDepth = ReconstructLinearDepth(textureDepth);

			float rangeCheck = smoothstep(0.0f, 1.0f, globalData.SSAOSettings.y / abs(textureDepth - sampledDepth));
			occlusion += (sampledDepth < textureDepth ? 1.0f : 0.0f) * rangeCheck;    
		}

		occlusion /= globalData.SSAOSettings.x;

		occlusion *= ssaoAvailableFactor;
	}

	outSSAOFactor = vec4(vec3(occlusion), 1.0);


	vec2 randomOffset = PDsrand2(vec2(perFrameData.time.x)) * 0.5f + 0.5f;
	vec2 noiseUV = (inUv + randomOffset) * globalData.gameWindowSize.xy * 0.5f;

	vec3 wsViewRay = normalize(inWsView);
	vec4 H;
	float RdotN = 0.0f;

	float regenCount = 0;
	for (; RdotN <= surfaceMargin && regenCount < maxRegenCount; regenCount++)
	{
		ivec3 inoiseUV = ivec3(ivec2(noiseUV + int(regenCount)) % 1024, pushConsts.blueNoiseTexIndex);
		vec2 Xi = texelFetch(RGBA8_1024_MIP_2DARRAY, inoiseUV, 0).rg;

		Xi.y = mix(Xi.y, 0.0f, globalData.SSRSettings0.x);	// Add a bias
		H = ImportanceSampleGGX(Xi, roughness);
		H.xyz = normalize(H.xyz);
		H.xyz = TBN * H.xyz;

		RdotN = dot(normal, reflect(wsViewRay, H.xyz));
	}

	outSSRInfo = RayMarch(H.xyz, normal, position, wsViewRay);

	// SSRInfo:
	// xy: hit position
	// z: pdf
	// sign(w): -1 not hit, 1 hit
	// w: step count
	outSSRInfo.a = outSSRInfo.a > 0.0f ? outSSRInfo.b : -outSSRInfo.b;
	outSSRInfo.b = H.w;
}