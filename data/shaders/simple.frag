#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inUv;
layout (location = 2) in vec3 inViewDir;
layout (location = 3) in vec3 inWorldPos;

layout (location = 0) out vec4 outFragColor;

const vec3 lightPos = vec3(-50, 0, 50);
const vec3 diffuseColor = vec3(1.0, 0.0, 0.0);
const vec3 lightColor = vec3(1);
const float roughness = 0.01;
const float gamma = 1.0 / 2.2;
const float PI = 3.14159265;
const float F0 = 0.04;

float GGX_D(float NdotH, float roughness)
{
	float m2 = roughness * roughness;
	float f = NdotH * NdotH * (m2 - 1) + 1;
	return m2 / (f * f * PI);
}

float GGX_V_Smith_HeightCorrelated(float NdotL, float NdotV, float roughness)
{
	float alpha2 = roughness * roughness;
	float NdotL2 = NdotL * NdotL;
	float NdotV2 = NdotV * NdotV;
	float lambdaV = NdotL * sqrt(alpha2 + (1 - alpha2) * NdotV2);
	float lambdaL = NdotV * sqrt(alpha2 + (1 - alpha2) * NdotL2);
	return 0.5f / (lambdaV + lambdaL);
}

float Fresnel_Schlick(float F0, float LdotH)
{
	return F0 + (1 - F0) * pow(1.0f - LdotH, 5.0f);
}

void main() 
{
	vec3 n = normalize(inNormal);
	vec3 v = normalize(inViewDir);
	vec3 l = normalize(lightPos - inWorldPos);
	vec3 h = normalize(l + v);

	float NdotH = max(0.0f, dot(n, h));
	float NdotL = max(0.0f, dot(n, l));
	float NdotV = max(0.0f, dot(n, v));
	float LdotH = max(0.0f, dot(l, h));

	float fresnel = Fresnel_Schlick(F0, LdotH);
	vec3 reflect = fresnel * GGX_V_Smith_HeightCorrelated(NdotL, NdotV, roughness) * GGX_D(NdotH, roughness) * lightColor;
	vec3 diffuse = diffuseColor * (1.0f - fresnel) / PI;
	vec3 final = (reflect + diffuse) * NdotL * lightColor;
	final = pow(final, vec3(gamma));

	outFragColor = vec4(final, 1.0);
}