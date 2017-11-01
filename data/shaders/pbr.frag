#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 3, binding = 1) uniform sampler2D albedoTex;
layout (set = 3, binding = 2) uniform sampler2D bumpTex;
layout (set = 3, binding = 3) uniform sampler2D roughnessTex;
layout (set = 3, binding = 4) uniform sampler2D metalicTex;
layout (set = 3, binding = 5) uniform sampler2D aoTex;
layout (set = 3, binding = 6) uniform samplerCube irradianceTex;
layout (set = 3, binding = 7) uniform samplerCube prefilterEnvTex;
layout (set = 3, binding = 8) uniform sampler2D BRDFLut;

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inWorldPos;
layout (location = 3) in vec3 inLightDir;
layout (location = 4) in vec3 inViewDir;
layout (location = 5) in vec3 inTangent;
layout (location = 6) in vec3 inBitangent;

layout (location = 0) out vec4 outFragColor;

const vec3 lightColor = vec3(1);
const float roughness = 1;
const float gamma = 1.0 / 2.2;
const float PI = 3.14159265;
vec3 F0 = vec3(0.04);
const float exposure = 4.5;
const float whiteScale = 11.2;

float GGX_D(float NdotH, float roughness)
{
	float m2 = roughness * roughness;
	float m4 = m2 * m2;
	float f = NdotH * NdotH * (m4 - 1) + 1;
	return m4 / (f * f * PI);
}

float GGX_V_Smith_HeightCorrelated(float NdotV, float NdotL, float roughness)
{
	float alpha2 = roughness * roughness;
	float lambdaV = NdotL * sqrt((-NdotV * alpha2 + NdotV) * NdotV + alpha2);
	float lambdaL = NdotV * sqrt((-NdotL * alpha2 + NdotL) * NdotL + alpha2);
	return 0.5f / (lambdaV + lambdaL) * 4.0f * NdotV * NdotL;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}

float GeometrySmith(float NdotV, float NdotL, float k)
{
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

vec3 Fresnel_Schlick_Roughness(vec3 F0, float LdotH, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0f - LdotH, 5.0f);
}

vec3 Fresnel_Schlick(vec3 F0, float LdotH)
{
	return F0 + (1.0 - F0) * pow(1.0 - LdotH, 5.0);
}

vec3 perturbNormal()
{
	vec3 tangentNormal = texture(bumpTex, inUv.st).xyz * 2.0 - 1.0;

	vec3 q1 = dFdx(inWorldPos);
	vec3 q2 = dFdy(inWorldPos);
	vec2 st1 = dFdx(inUv.st);
	vec2 st2 = dFdy(inUv.st);

	vec3 N = normalize(inNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

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
	vec3 pertNormal = texture(bumpTex, inUv.st, 0.0).rgb;
	pertNormal = pertNormal * 2.0 - vec3(1.0);

	mat3 TBN = mat3(inTangent, inBitangent, inNormal);
	pertNormal = TBN * pertNormal;

	vec3 n = normalize(pertNormal);
	//vec3 n = normalize(inNormal);
	vec3 v = normalize(inViewDir);
	vec3 l = normalize(inLightDir);
	vec3 h = normalize(l + v);

	float NdotH = max(0.0f, dot(n, h));
	float NdotL = max(0.0f, dot(n, l));
	float NdotV = max(0.0f, dot(n, v));
	float LdotH = max(0.0f, dot(l, h));

	vec3 albedo = pow(texture(albedoTex, inUv.st, 0.0).rgb, vec3(2.2));
	vec3 normal = texture(bumpTex, inUv.st, 0.0).rgb;
	float roughness = texture(roughnessTex, inUv.st, 0.0).r;
	float metalic = texture(metalicTex, inUv.st, 0.0).r;
	float ao = texture(aoTex, inUv.st, 0.0).r;
	F0 = mix(F0, albedo, metalic);

	vec3 fresnel = Fresnel_Schlick(F0, LdotH);
	vec3 kD = (1.0 - metalic) * (vec3(1.0) - fresnel);

	//-------------- Ambient -----------------------
	vec3 fresnel_roughness = Fresnel_Schlick_Roughness(F0, NdotV, roughness);
	vec3 kD_roughness = (1.0 - metalic) * (vec3(1.0) - fresnel_roughness);

	vec3 irradiance = texture(irradianceTex, vec3(n.x, -n.y, n.z)).rgb * albedo / PI;

	//vec3 reflectSampleDir = NdotV * n * 2.0 - v;
	//reflectSampleDir.y *= -1.0;
	vec3 reflectSampleDir = reflect(-v, n);
	reflectSampleDir.y *= -1.0;

	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = roughness * MAX_REFLECTION_LOD;
	vec3 reflect = textureLod(prefilterEnvTex, reflectSampleDir, lod).rgb;
	vec2 brdf_lut = texture(BRDFLut, vec2(NdotV, roughness)).rg;

	// Here we use NdotV rather than LdotH, since L's direction is based on punctual light, and here ambient reflection calculation
	// requires reflection vector dot with N, which is RdotN, equals NdotV
	vec3 ambient = (reflect * (brdf_lut.x * fresnel_roughness + brdf_lut.y) + irradiance * kD_roughness) * ao;
	//----------------------------------------------

	vec3 dirLightSpecular = fresnel * GGX_V_Smith_HeightCorrelated(NdotV, NdotL, roughness) * GGX_D(NdotH, roughness);
	vec3 dirLightDiffuse = albedo * kD / PI;
	vec3 final = ao * ((dirLightSpecular + dirLightDiffuse) * NdotL * lightColor) + ambient;

	final = Uncharted2Tonemap(final * exposure);
	final = final * (1.0 / Uncharted2Tonemap(vec3(whiteScale)));
	final = pow(final, vec3(gamma));

	outFragColor = vec4(final, 1.0);
}