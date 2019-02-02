#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D MotionVector[3];
layout (set = 3, binding = 3) uniform sampler2D ShadingResult[3];
layout (set = 3, binding = 4) uniform sampler2D GBuffer3[3];
layout (set = 3, binding = 5) uniform sampler2D MotionNeighborMax[3];
layout (set = 3, binding = 6) uniform sampler2D TemporalResult[2];
layout (set = 3, binding = 7) uniform sampler2D TemporalCoC[2];

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec2 inOneNearPosition;

layout (location = 0) out vec4 outTemporalResult;
layout (location = 1) out vec4 outTemporalCoC;

int index = int(perFrameData.camDir.a + 0.5f);
int pingpong = int(perFrameData.camPos.a + 0.5f);

float prevMotionImpactAmp = globalData.TemporalSettings0.x;
float currMotionImpactAmp = globalData.TemporalSettings0.y;
float maxClippedPrevRatio = globalData.TemporalSettings0.z;
float motionImpactLowerBound = globalData.TemporalSettings1.x;	// FIXME: This should be impact by frame rate
float motionImpactUpperBound = globalData.TemporalSettings1.y;	// FIXME: This should be impact by frame rate

vec4 ResolveColor(sampler2D currSampler, sampler2D prevSampler, vec2 unjitteredUV, vec2 motionVec, float motionNeighborMaxLength)
{
	float prevMotionLen = texture(prevSampler, unjitteredUV).a;
	float currMaxMotionLen = max(motionNeighborMaxLength, prevMotionLen);
	float prevMotionImpact = abs(prevMotionLen - motionNeighborMaxLength);

	vec3 curr = texture(currSampler, unjitteredUV).rgb;
	vec3 prev = texture(prevSampler, inUv + motionVec).rgb;

	vec2 u = vec2(globalData.gameWindowSize.z, 0);
	vec2 v = vec2(0, globalData.gameWindowSize.w);

	vec3 bl = texture(currSampler, unjitteredUV - u - v).rgb;
	vec3 bm = texture(currSampler, unjitteredUV - v).rgb;
	vec3 br = texture(currSampler, unjitteredUV + u - v).rgb;
	vec3 ml = texture(currSampler, unjitteredUV - u).rgb;
	vec3 mr = texture(currSampler, unjitteredUV + u).rgb;
	vec3 tl = texture(currSampler, unjitteredUV - u + v).rgb;
	vec3 tm = texture(currSampler, unjitteredUV + v).rgb;
	vec3 tr = texture(currSampler, unjitteredUV + u + v).rgb;

	vec3 minColor = min(bl, min(bm, min(br, min(ml, min(mr, min(tl, min(tm, min(tr, curr))))))));
	vec3 maxColor = max(bl, max(bm, max(br, max(ml, max(mr, max(tl, max(tm, max(tr, curr))))))));

	// min max suppress to reduce temporal ghosting effect
	vec3 clippedPrev = clip_aabb(minColor, maxColor, prev);

	float prevMix = 1.0f - smoothstep(motionImpactLowerBound, motionImpactUpperBound, currMaxMotionLen);
	prev = mix(clippedPrev, prev, mix(max(0.0f, maxClippedPrevRatio - prevMotionImpact * prevMotionImpactAmp), 1.0f, prevMix));
	//prev = mix(clippedPrev, prev, mix(max(0.0f, maxClippedPrevRatio - prevMotionImpact * prevMotionImpactAmp), 1.0f, prevMix));

	float currLum = Luminance(curr);
	float prevLum = Luminance(prev);

	// No point doing this
	//float unbiasedDiff = abs(currLum - prevLum) / max(currLum, max(prevLum, 0.2f));
	//float unbiasedWeight = 1.0 - unbiasedDiff;
	//float unbiasedWeightSQR = unbiasedWeight * unbiasedWeight;
	//float feedback = mix(0.87f, 0.97f, unbiasedWeightSQR);

	return vec4(mix(curr, prev, 0.98f), motionNeighborMaxLength);
}

float ResolveCoC(sampler2D currSampler, sampler2D prevSampler, sampler2D motionVecSampler, vec2 unjitteredUV)
{
	vec3 offset = globalData.gameWindowSize.zww * vec3(1, 1, 0);

	float coc1 = texture(currSampler, unjitteredUV - offset.xz).a;
	float coc2 = texture(currSampler, unjitteredUV - offset.zy).a;
	float coc3 = texture(currSampler, unjitteredUV + offset.zy).a;
	float coc4 = texture(currSampler, unjitteredUV + offset.xz).a;

	float coc0 = texture(currSampler, inUv).a;

	// Dilation
	vec3 closest = vec3(0, 0, coc0);
	closest = coc1 < closest.z ? vec3(-offset.xz, coc1) : closest;
	closest = coc2 < closest.z ? vec3(-offset.zy, coc2) : closest;
	closest = coc3 < closest.z ? vec3(offset.zy, coc3) : closest;
	closest = coc4 < closest.z ? vec3(offset.xz, coc4) : closest;

	float minCoC = min(coc0, min(coc1, min(coc2, min(coc3, coc4))));
	float maxCoC = max(coc0, max(coc1, max(coc2, max(coc3, coc4))));

	vec2 motionVec = texture(motionVecSampler, unjitteredUV + closest.xy).xy;

	float prevCoC = texture(prevSampler, inUv + motionVec).r;
	prevCoC = clamp(prevCoC, minCoC, maxCoC);

	return mix(coc0, prevCoC, 0.98f);
}

void main() 
{
	vec2 unjitteredUV = inUv - perFrameData.cameraJitterOffset;
	
	vec2 motionVec = texture(MotionVector[index], unjitteredUV).rg;
	vec2 motionNeighborMaxFetch = abs(texelFetch(MotionNeighborMax[index], ivec2(unjitteredUV * globalData.motionTileWindowSize.zw), 0).rg);
	vec2 motionNeighborMaxSample = abs(texture(MotionNeighborMax[index], unjitteredUV).rg);
	vec2 diff = abs(motionNeighborMaxFetch - motionNeighborMaxSample);

	// Using sampled value to expend tile area by hardware interpolation, so that ghosting will be reduced due to edge precision in tile boundary
	float motionNeighborMaxLength = length(motionNeighborMaxFetch + diff) * currMotionImpactAmp;

	outTemporalResult = ResolveColor(ShadingResult[index], TemporalResult[pingpong], unjitteredUV, motionVec, motionNeighborMaxLength);
	outTemporalCoC = vec4(ResolveCoC(GBuffer3[index], TemporalCoC[pingpong], MotionVector[index], unjitteredUV));
}