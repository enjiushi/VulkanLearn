#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

layout (set = 3, binding = 2) uniform sampler2D TemporalResult[2];
layout (set = 3, binding = 3) uniform sampler2D TemporalCoC[2];

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outPrefilter;

int index = int(perFrameData.camDir.a);
int pingpong = (int(perFrameData.camPos.a) + 1) % 2;

void main() 
{
	vec3 offset = globalData.gameWindowSize.zwz * vec3(0.5f, 0.5f, -0.5f);

	vec2 uv0 = inUv - offset.xy;
	vec2 uv1 = inUv - offset.zy;
	vec2 uv2 = inUv + offset.zy;
	vec2 uv3 = inUv + offset.xy;
	
	vec3 color0 = texture(TemporalResult[pingpong], uv0).rgb;
	vec3 color1 = texture(TemporalResult[pingpong], uv1).rgb;
	vec3 color2 = texture(TemporalResult[pingpong], uv2).rgb;
	vec3 color3 = texture(TemporalResult[pingpong], uv3).rgb;

	float coc0 = texture(TemporalCoC[pingpong], uv0).r * 2.0f - 1.0f;
	float coc1 = texture(TemporalCoC[pingpong], uv1).r * 2.0f - 1.0f;
	float coc2 = texture(TemporalCoC[pingpong], uv2).r * 2.0f - 1.0f;
	float coc3 = texture(TemporalCoC[pingpong], uv3).r * 2.0f - 1.0f;

	float w0 = abs(coc0) / (max(color0.r, max(color0.g, color0.b)) + 1.0f);
	float w1 = abs(coc1) / (max(color1.r, max(color1.g, color1.b)) + 1.0f);
	float w2 = abs(coc2) / (max(color2.r, max(color2.g, color2.b)) + 1.0f);
	float w3 = abs(coc3) / (max(color3.r, max(color3.g, color3.b)) + 1.0f);

	vec3 avg = color0 * w0 + color1 * w1 + color2 * w2 + color3 * w3;
	avg /= max(w0 + w1 + w2 + w3, 1e-5);

	float minCoC = min(coc0, min(coc1, min(coc2, coc3)));
	float maxCoC = max(coc0, max(coc1, max(coc2, coc3)));
	float coc = (-minCoC > maxCoC ? minCoC : maxCoC) * globalData.DOFSettings0.x;

	avg *= smoothstep(0, globalData.gameWindowSize.z * 2.0f, abs(coc));

	outPrefilter = vec4(avg, coc);
}