#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 3, binding = 0) uniform samplerCube envTex;

#include "uniform_layout.h"

layout (location = 0) in vec3 inSampleDir;

layout (location = 0) out vec4 outFragColor;

const float exposure = 4.5;
const float whiteScale = 11.2;
const float gamma = 1.0 / 2.2;

vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main() 
{
	vec3 final = texture(envTex, normalize(inSampleDir)).xyz;
	
	final = Uncharted2Tonemap(final * globalData.GEW.y);
	final = final * (1.0 / Uncharted2Tonemap(vec3(globalData.GEW.z)));
	final = pow(final, vec3(globalData.GEW.x));

	outFragColor = vec4(final, 1.0);
}