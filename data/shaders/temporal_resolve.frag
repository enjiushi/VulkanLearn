#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D MotionVector[3];
layout (set = 3, binding = 3) uniform sampler2D ShadingResult[3];
layout (set = 3, binding = 4) uniform sampler2D MotionNeighborMax[3];
layout (set = 3, binding = 5) uniform sampler2D TemporalResult[2];

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec2 inOneNearPosition;

layout (location = 0) out vec4 outTemporalResult;

int index = int(perFrameData.camDir.a);
int pingpong = int(perFrameData.camPos.a);

//float prevMotionImpactAmp = 6.7f;
	//float currMotionImpactAmp = 50.0f;
	//float maxClippedPrevRatio = 0.8f;
	//float motionImpactLowerBound = 0.0005f;
	//float motionImpactUpperBound = 0.001f;

	float prevMotionImpactAmp = globalData.TemporalSettings0.x;
	float currMotionImpactAmp = globalData.TemporalSettings0.y;
	float maxClippedPrevRatio = globalData.TemporalSettings0.z;
	float motionImpactLowerBound = globalData.TemporalSettings1.x;	// FIXME: This should be impact by frame rate
	float motionImpactUpperBound = globalData.TemporalSettings1.y;	// FIXME: This should be impact by frame rate

vec4 resolve(sampler2D currSampler, sampler2D prevSampler, vec2 unjitteredUV, vec2 motionVec, float motionNeighborMaxLength)
{
	float prevMotionLen = texture(prevSampler, unjitteredUV).a;
	float currMaxMotionLen = max(motionNeighborMaxLength, prevMotionLen);
	float prevMotionImpact = clamp(0.0f, 999.0f, prevMotionLen - motionNeighborMaxLength);

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

	float currLum = Luminance(curr);
	float prevLum = Luminance(prev);

	// No point doing this
	//float unbiasedDiff = abs(currLum - prevLum) / max(currLum, max(prevLum, 0.2f));
	//float unbiasedWeight = 1.0 - unbiasedDiff;
	//float unbiasedWeightSQR = unbiasedWeight * unbiasedWeight;
	//float feedback = mix(0.87f, 0.97f, unbiasedWeightSQR);

	return vec4(mix(curr, prev, 0.98f), motionNeighborMaxLength);
}

void main() 
{
	vec2 unjitteredUV = inUv - perFrameData.cameraJitterOffset;
	
	vec2 motionVec = texture(MotionVector[index], unjitteredUV).rg;
	vec2 motionNeighborMaxFetch = texelFetch(MotionNeighborMax[index], ivec2(unjitteredUV * globalData.motionTileWindowSize.zw), 0).rg;
	vec2 motionNeighborMaxSample = texture(MotionNeighborMax[index], unjitteredUV).rg;
	vec2 diff = abs(motionNeighborMaxFetch - motionNeighborMaxSample);

	// Using sampled value to expend tile area by hardware interpolation, so that ghosting will be reduced due to edge precision in tile boundary
	float motionNeighborMaxLength = length(motionNeighborMaxFetch + diff) * currMotionImpactAmp;

	outTemporalResult = resolve(ShadingResult[index], TemporalResult[pingpong], unjitteredUV, motionVec, motionNeighborMaxLength);
}