#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D MotionVector[3];
layout (set = 3, binding = 3) uniform sampler2D ShadingResult[3];
layout (set = 3, binding = 4) uniform sampler2D TemporalResult[2];

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec2 inOneNearPosition;

layout (location = 0) out vec4 outTemporalResult;

int index = int(perFrameData.camDir.a);
int pingpong = int(perFrameData.camPos.a);

vec3 resolve(sampler2D currSampler, sampler2D prevSampler, vec2 unjitteredUV, vec2 motionVec)
{
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

	outTemporalResult = vec4(resolve(ShadingResult[index], TemporalResult[pingpong], unjitteredUV, motionVec), 1.0f);
}