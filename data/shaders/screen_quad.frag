#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 3, binding = 1) uniform samplerCube tex0;
layout (set = 3, binding = 2) uniform sampler2D tex1;

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outFragColor;

const float exposure = 2.0;
const float whiteScale = 11.0;
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
	//vec3 final = texture(tex1, inUv.st).xyz;
	vec3 final = texture(tex0, normalize(vec3((inUv.st - 0.5) * 2.0, 1.0)), 3.0).xyz;
	
	final = Uncharted2Tonemap(final * exposure);
	final = final * (1.0 / Uncharted2Tonemap(vec3(whiteScale)));
	final = pow(final, vec3(gamma));

	outFragColor = vec4(final, 1.0);
	//outFragColor = vec4(1, 0, 0, 1.0);
}