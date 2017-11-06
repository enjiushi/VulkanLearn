#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 3, binding = 0) uniform samplerCube envTex;

layout (location = 0) in vec3 inSampleDir;

layout (location = 0) out vec4 outFragColor;

#include "uniform_layout.h"

const vec3 up = {0.0, 1.0, 0.0};
const float PI = 3.14159265;
const uint numSamples = 1024;

// https://learnopengl.com/#!PBR/IBL/Specular-IBL
float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// https://learnopengl.com/#!PBR/IBL/Specular-IBL
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
} 

// https://learnopengl.com/#!PBR/IBL/Specular-IBL
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

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
		vec3 L = ImportanceSampleGGX(Xi, N, perFrameData.camPos.w);

		float NdotL = dot(N, L);
		if (NdotL > 0)
		{
			color += texture(envTex, L).rgb;
			weight += NdotL;
		}
	}
	outFragColor = vec4(color / weight, 1.0);
}