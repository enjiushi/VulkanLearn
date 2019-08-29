#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

layout (set = 3, binding = 2) uniform sampler2D MotionVector[3];
layout (set = 3, binding = 3) uniform sampler2D ShadingResult[3];
layout (set = 3, binding = 4) uniform sampler2D SSRResult[3];
layout (set = 3, binding = 5) uniform sampler2D GBuffer1[3];
layout (set = 3, binding = 6) uniform sampler2D MotionNeighborMax[3];
layout (set = 3, binding = 7) uniform sampler2D TemporalShadingResult;
layout (set = 3, binding = 8) uniform sampler2D TemporalSSRResult;
layout (set = 3, binding = 9) uniform sampler2D TemporalResult;
layout (set = 3, binding = 10) uniform sampler2D TemporalCoC;

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec2 inOneNearPosition;

layout (location = 0) out vec4 outTemporalShadingResult;
layout (location = 1) out vec4 outTemporalSSRResult;
layout (location = 2) out vec4 outTemporalResult;
layout (location = 3) out vec4 outTemporalCoC;

int index = int(perFrameData.camDir.a + 0.5f);
int pingpong = int(perFrameData.camPos.a + 0.5f);

float motionImpactLowerBound = globalData.TemporalSettings0.x;
float motionImpactUpperBound = globalData.TemporalSettings0.y;

vec4 ResolveShadingResult(sampler2D currSampler, sampler2D prevSampler, vec2 unjitteredUV, vec2 motionVec)
{
	vec4 curr = texture(currSampler, unjitteredUV);
	vec4 prev = texture(prevSampler, inUv + motionVec);

	vec2 u = vec2(globalData.gameWindowSize.z, 0);
	vec2 v = vec2(0, globalData.gameWindowSize.w);

	vec4 bl = texture(currSampler, unjitteredUV - u - v);
	vec4 bm = texture(currSampler, unjitteredUV - v);
	vec4 br = texture(currSampler, unjitteredUV + u - v);
	vec4 ml = texture(currSampler, unjitteredUV - u);
	vec4 mr = texture(currSampler, unjitteredUV + u);
	vec4 tl = texture(currSampler, unjitteredUV - u + v);
	vec4 tm = texture(currSampler, unjitteredUV + v);
	vec4 tr = texture(currSampler, unjitteredUV + u + v);

	vec4 minColor = min(bl, min(bm, min(br, min(ml, min(mr, min(tl, min(tm, min(tr, curr))))))));
	vec4 maxColor = max(bl, max(bm, max(br, max(ml, max(mr, max(tl, max(tm, max(tr, curr))))))));

	// min max suppress to reduce temporal ghosting effect
	vec3 clippedPrev = clip_aabb(minColor.rgb, maxColor.rgb, prev.rgb);

	float currLum = Luminance(curr.rgb);
	float prevLum = Luminance(prev.rgb);

	float unbiasedDiff = abs(currLum - prevLum) / max(currLum, max(prevLum, 0.2f));
	float unbiasedWeight = 1.0 - unbiasedDiff;
	float unbiasedWeightSQR = unbiasedWeight * unbiasedWeight;
	float feedback = mix(0.87f, 0.97f, unbiasedWeightSQR);

	return vec4(mix(curr.rgb, clippedPrev, feedback), 1.0f);
}

vec4 ResolveSSRResult(sampler2D currSampler, sampler2D prevSampler, vec2 unjitteredUV, vec2 motionVec, float currMotion)
{
	vec4 curr = texture(currSampler, unjitteredUV);
	vec4 prev = texture(prevSampler, inUv + motionVec);

	float currSSRMask = curr.a;
	float prevMotion = prev.a;

	vec2 u = vec2(globalData.gameWindowSize.z, 0);
	vec2 v = vec2(0, globalData.gameWindowSize.w);

	vec4 bl = texture(currSampler, unjitteredUV - u - v);
	vec4 bm = texture(currSampler, unjitteredUV - v);
	vec4 br = texture(currSampler, unjitteredUV + u - v);
	vec4 ml = texture(currSampler, unjitteredUV - u);
	vec4 mr = texture(currSampler, unjitteredUV + u);
	vec4 tl = texture(currSampler, unjitteredUV - u + v);
	vec4 tm = texture(currSampler, unjitteredUV + v);
	vec4 tr = texture(currSampler, unjitteredUV + u + v);

	vec4 minColor = min(bl, min(bm, min(br, min(ml, min(mr, min(tl, min(tm, min(tr, curr))))))));
	vec4 maxColor = max(bl, max(bm, max(br, max(ml, max(mr, max(tl, max(tm, max(tr, curr))))))));

	vec4 clippedPrev;
	clippedPrev.rgb = clip_aabb(minColor.rgb, maxColor.rgb, prev.rgb);
	clippedPrev.a = clamp(prev.a, minColor.a, maxColor.a);

	vec4 lowResponseSSR = mix(curr, prev, 0.98f);
	vec4 highResponseSSR = mix(curr, clippedPrev, 0.98f);
	highResponseSSR = mix(lowResponseSSR, highResponseSSR, 0.7f);

	float currMotionFactor = currMotion / motionImpactUpperBound;
	float prevMotionFactor = prevMotion / motionImpactUpperBound;
	float ssrFactor = 1.0f - min(currSSRMask, 0.000001f) / 0.000001f;

	float factor = min(ssrFactor + currMotionFactor + prevMotionFactor, 1.0f);

	return vec4(mix(lowResponseSSR.rgb, highResponseSSR.rgb, factor), currMotion);
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

	outTemporalShadingResult = ResolveShadingResult(ShadingResult[index], TemporalShadingResult, unjitteredUV, motionVec);
	outTemporalSSRResult = ResolveSSRResult(SSRResult[index], TemporalSSRResult, unjitteredUV, motionVec, length(motionNeighborMaxFetch));
	outTemporalCoC = vec4(ResolveCoC(GBuffer1[index], TemporalCoC, MotionVector[index], unjitteredUV));

	outTemporalResult = outTemporalShadingResult + outTemporalSSRResult;
}