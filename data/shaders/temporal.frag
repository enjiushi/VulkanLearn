#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout(push_constant) uniform PushConsts {
	layout (offset = 0) int pingpong;
} pushConsts;

layout (set = 3, binding = 2) uniform sampler2D ShadingResult[3];
layout (set = 3, binding = 3) uniform sampler2D MotionVector[3];
layout (set = 3, binding = 4) uniform sampler2D TemporalHistory;

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outFragColor0;

const float FLT_EPS = 0.00000001f;

float Luminance(in vec3 color)
{
    return dot(color, vec3(0.2126f, 0.7152f, 0.0722f));
}

vec3 clip_aabb(vec3 aabb_min, vec3 aabb_max, vec3 p)
{
	// note: only clips towards aabb center (but fast!)
	vec3 p_clip = 0.5f * (aabb_max + aabb_min);
	vec3 e_clip = 0.5f * (aabb_max - aabb_min) + FLT_EPS;

	vec3 v_clip = p - p_clip;
	vec3 v_unit = v_clip / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if (ma_unit > 1.0f)
		return p_clip + v_clip / ma_unit;
	else
		return p;// point inside aabb
}

void main() 
{
	int index = int(perFrameData.camDir.a);

	vec2 unjitteredUV = inUv - perFrameData.cameraJitterOffset;
	
	vec2 motionVec = texture(MotionVector[index], unjitteredUV).rg;

	vec3 curr = texture(ShadingResult[index], unjitteredUV).rgb;
	vec3 prev = texture(TemporalHistory, inUv + motionVec).rgb;

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
	prev = clip_aabb(minColor, maxColor, prev);

	float currLum = Luminance(curr);
	float prevLum = Luminance(prev);

	float unbiasedDiff = abs(currLum - prevLum) / max(currLum, max(prevLum, 0.2f));
	float unbiasedWeight = 1.0 - unbiasedDiff;
	float unbiasedWeightSQR = unbiasedWeight * unbiasedWeight;
	float feedback = mix(0.87f, 0.97f, unbiasedWeightSQR);

	vec3 final = mix(curr, prev, feedback);

	outFragColor0 = vec4(final, 1.0);
}