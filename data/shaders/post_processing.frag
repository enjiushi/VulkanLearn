#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D TemporalHistory;
layout (set = 3, binding = 3) uniform sampler2D BloomTextures[3];
layout (set = 3, binding = 4) uniform sampler2D MotionVector[3];

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outFragColor0;

int index = int(perFrameData.camDir.a);

const float bloomMagnitude = 0.2f;
const float bloomExposure = 1.3f;

const float MOTION_VEC_AMP = 5.0f;
const float MOTION_VEC_SAMPLE_COUNT = 16;

void main() 
{
	ivec2 coord = ivec2(floor(inUv * globalData.gameWindowSize.xy));

	vec3 final = vec3(0);

	vec2 motionVec = texelFetch(MotionVector[index], coord, 0).rg;
	vec2 step = motionVec / MOTION_VEC_SAMPLE_COUNT * MOTION_VEC_AMP;
	ivec2 istep = ivec2(floor(step * globalData.gameWindowSize.xy));
	ivec2 offset;

	final = texelFetch(TemporalHistory, coord, 0).rgb;
	final += pow(texture(BloomTextures[index], inUv).rgb * bloomMagnitude, vec3(bloomExposure));

	final = Uncharted2Tonemap(final * globalData.GEW.y);
	final = final * (1.0 / Uncharted2Tonemap(vec3(globalData.GEW.z)));
	final = pow(final, vec3(globalData.GEW.x));

	outFragColor0 = vec4(final, 1.0);
}