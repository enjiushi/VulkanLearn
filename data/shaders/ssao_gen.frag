#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D GBuffer0[3];
layout (set = 3, binding = 3) uniform sampler2D DepthStencilBuffer[3];

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inViewRay;

layout (location = 0) out vec4 outFragColor0;

int index = int(perFrameData.camDir.a);

float SSAO_RADIUS = 15.0f;

vec3 UnpackNormal(ivec2 coord)
{
	vec4 gbuffer0 = texelFetch(GBuffer0[index], coord, 0);

	return normalize(gbuffer0.xyz * 2.0f - 1.0f);
}

void main() 
{
	ivec2 coord = ivec2(floor(inUv * globalData.gameWindowSize.xy));

	vec3 normal = UnpackNormal(coord);

	float linearDepth;
	vec3 position = ReconstructPosition(coord, inViewRay, DepthStencilBuffer[index], linearDepth);

	vec3 tangent = texture(SSAO_RANDOM_ROTATIONS, inUv * globalData.SSAOWindowSize.xy / textureSize(SSAO_RANDOM_ROTATIONS, 0)).xyz * 2.0f - 1.0f;
	tangent = normalize(tangent - dot(normal, tangent) * normal);

	vec3 bitangent = normalize(cross(normal, tangent));

	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0f;

	for (int i = 0; i < 64; i++)
	{
		vec3 sampleDir = TBN * globalData.SSAOSamples[i].xyz;
		vec3 samplePos = position + sampleDir * SSAO_RADIUS;

		vec4 clipSpaceSample = perFrameData.VPN * vec4(samplePos, 1.0f);
		clipSpaceSample = clipSpaceSample / clipSpaceSample.w;
		clipSpaceSample.xy = clipSpaceSample.xy * 0.5f + 0.5f;

		float sampledDepth = clipSpaceSample.z;
		float textureDepth = texture(DepthStencilBuffer[index], clipSpaceSample.xy).r;

		sampledDepth = ReconstructLinearDepth(sampledDepth);
		textureDepth = ReconstructLinearDepth(textureDepth);

		float rangeCheck = smoothstep(0.0f, 1.0f, SSAO_RADIUS / abs(textureDepth - sampledDepth));
		occlusion += (sampledDepth < textureDepth ? 1.0f : 0.0f) * rangeCheck;    
	}

	occlusion /= 64.0f;

	outFragColor0 = vec4(vec3(occlusion), 1.0);
}