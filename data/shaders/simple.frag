#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inUv;
layout (location = 2) in vec3 inViewDir;

layout (location = 0) out vec4 outFragColor;

const vec3 lightDir = vec3(0.5);
const vec3 diffuseColor = vec3(1.0, 0.0, 0.0);
const vec3 lightColor = vec3(1.0);
const float shiness = 512.0;
const float gamma = 1.0 / 2.2;

void main() 
{
	vec3 n = normalize(inNormal);
	vec3 v = normalize(inViewDir);

	vec3 diff = max(dot(n, lightDir), 0.0) * diffuseColor;

	vec3 h = normalize(lightDir + v);
	vec3 spec = pow(max(dot(h, n), 0.0), shiness) * lightColor;

	vec3 final = pow(diff + spec, vec3(gamma));
	outFragColor = vec4(final, 1.0);
}