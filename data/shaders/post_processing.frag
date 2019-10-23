#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

layout (set = 3, binding = 3) uniform sampler2D CombineResult[3];
layout (set = 3, binding = 4) uniform sampler2D MotionNeighborMax[3];

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outScreen;

int index = int(perFrameData.camDir.a);

float vignetteMinDist = globalData.VignetteSettings.x;
float vignetteMaxDist = globalData.VignetteSettings.y;
float vignettAmp = globalData.VignetteSettings.z;

void main() 
{
	float motionAmp = globalData.MotionBlurSettings.x * perFrameData.time.x;

	vec3 noneMotionColor = texture(CombineResult[index], inUv).rgb;

	// Motion Blur
	vec3 fullMotionColor = vec3(0);
	vec2 motionNeighborMax = texture(MotionNeighborMax[index], inUv).rg;
	vec2 step = motionNeighborMax / globalData.MotionBlurSettings.y;	// either side samples a pre-defined amount of colors
	vec2 startPos = inUv + step * 0.5f * PDsrand(inUv + vec2(perFrameData.time.y));	// Randomize starting position

	for (int i = int(-globalData.MotionBlurSettings.y / 2.0f); i <= int(globalData.MotionBlurSettings.y / 2.0f); i++)
	{
		fullMotionColor += texture(CombineResult[index], startPos + step * i).rgb;
	}

	fullMotionColor /= globalData.MotionBlurSettings.y;

	const float noneMotion = 2.0f;
	const float fullMotion = 15.0f;
	const float span = fullMotion - noneMotion;
	
	float motionMag = length(motionNeighborMax * globalData.gameWindowSize.xy) * motionAmp;
	float motionMix = clamp(motionMag - noneMotion, 0.0f, span) / span;
	vec3 final = mix(noneMotionColor, fullMotionColor, motionMix);

	// Vignette
	vec2 center = vec2(0.5f, 0.5f);
	float distToCenter = abs(length(inUv - center));
	float vignetteFactor = max(0.0f, 1.0f - smoothstep(vignetteMinDist, vignetteMaxDist, distToCenter) * vignettAmp);
	final *= vignetteFactor;

	// Chromatic Abberation

	final = Uncharted2Tonemap(final * globalData.GEW.y);
	final = final * (1.0 / Uncharted2Tonemap(vec3(globalData.GEW.z)));
	final = pow(final, vec3(globalData.GEW.x));

	outScreen = vec4(final, 1.0);
}