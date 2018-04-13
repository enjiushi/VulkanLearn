struct GlobalData
{
	// Camera Settings
	mat4 projection;
	mat4 vulkanNDC;
	mat4 PN;

	// Scene Settings
	vec4 mainLightDir;
	vec4 mainLightColor;
	mat4 mainLightVPN;

	// Render Settings
	vec4 GEW;		//Gamma, exposure, white scale

	// SSAO Settings
	vec4 SSAOSamples[16];
};

struct PerFrameData
{
	mat4 view;
	mat4 viewCoordSystem;
	mat4 VPN;
	vec4 camPos;
	vec4 camDir;
	vec4 eyeSpaceSize;
	vec4 nearFarAB;
};

struct PerObjectData
{
	mat4 model;
	mat4 MVPN;
};

struct ObjectDataIndex
{
	int perObjectIndex;
	int perMaterialIndex;
};

layout(set = 0, binding = 0) uniform GlobalUniforms
{
	GlobalData		globalData;
};

layout(set = 0, binding = 1) uniform sampler2DArray RGBA8_1024_MIP_2DARRAY;
layout(set = 0, binding = 2) uniform sampler2DArray R8_1024_MIP_2DARRAY;
layout(set = 0, binding = 3) uniform samplerCube RGBA16_1024_MIP_CUBE_SKYBOX;
layout(set = 0, binding = 4) uniform samplerCube RGBA16_512_CUBE_IRRADIANCE;
layout(set = 0, binding = 5) uniform samplerCube RGBA16_512_CUBE_PREFILTERENV;
layout(set = 0, binding = 6) uniform sampler2D RGBA16_512_2D_BRDFLUT;
layout(set = 0, binding = 7) uniform sampler2D SSAO_RANDOM_ROTATIONS;

layout(set = 1, binding = 0) uniform PerFrameUniforms
{
	PerFrameData perFrameData;
};

layout(set = 2, binding = 0) buffer PerObjectUniforms
{
	PerObjectData perObjectData[];
};

layout(set = 3, binding = 0) buffer PerMaterialIndirectUniforms
{
	ObjectDataIndex objectDataIndex[];
};

const float PI = 3.14159265;
vec3 F0 = vec3(0.04);
const vec3 up = { 0.0, 1.0, 0.0 };

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

vec3 Fresnel_Schlick_Roughness(vec3 F0, float NdotV, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0f - NdotV, 5.0f);
}

vec3 Fresnel_Schlick(vec3 F0, float LdotH)
{
	return F0 + (1.0 - F0) * pow(1.0 - LdotH, 5.0);
}

float G_SchlicksmithGGX(float NdotL, float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = NdotL / (NdotL * (1.0 - k) + k);
	float GV = NdotV / (NdotV * (1.0 - k) + k);
	return GL * GV;
}

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
	return vec2(float(i) / float(N), RadicalInverse_VdC(i));
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
	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x + C*B) + D*E) / (x*(A*x + B) + D*F)) - E / F;
}