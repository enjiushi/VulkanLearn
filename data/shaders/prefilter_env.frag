#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inSampleDir;

layout (location = 0) out vec4 outFragColor;

#include "uniform_layout.sh"

const uint numSamples = 1024;

void main() 
{
	vec3 N = normalize(inSampleDir);
	vec3 R = N;
	vec3 V = N;

	float weight = 0;
	vec3 color = vec3(0);

	for (int samples = 0; samples < numSamples; samples++)
	{
		vec2 Xi = Hammersley(samples, numSamples);
		vec3 L = ImportanceSampleGGX(Xi, N, perFrameData.reservedPadding0);

		float NdotL = dot(N, L);
		if (NdotL > 0)
		{
			color += texture(RGBA16_1024_MIP_CUBE_SKYBOX, L).rgb;
			weight += NdotL;
		}
	}
	outFragColor = vec4(color / weight, 1.0);
}