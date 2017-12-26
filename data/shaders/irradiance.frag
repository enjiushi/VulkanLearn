#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 3, binding = 1) uniform samplerCube envTex;

layout (location = 0) in vec3 inSampleDir;

layout (location = 0) out vec4 outFragColor;

#include "uniform_layout.h"

const float sampleDelta = 0.15;

void main() 
{
	vec3 N = normalize(inSampleDir);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);

	vec3 zAxis = normalize(inSampleDir);
	vec3 xAxis = normalize(cross(up, zAxis));
	vec3 yAxis = normalize(cross(zAxis, xAxis));
	mat3 tangentSpace = mat3(xAxis, yAxis, zAxis);

	float numSamples = 0;

	vec3 irradiance = vec3(0.0);
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
			vec3 sampleDir = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			sampleDir = normalize(tangentSpace * sampleDir);
			irradiance += texture(RGBA16_1024_MIP_CUBE_SKYBOX, sampleDir).rgb * cos(theta) * sin(theta);
			numSamples++;
		}
	}

	vec3 final = irradiance / numSamples * PI;
	outFragColor = vec4(final, 1.0);
}