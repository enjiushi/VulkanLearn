#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (input_attachment_index = 0, set = 3, binding = 2) uniform subpassInput GBuffer0;
layout (input_attachment_index = 1, set = 3, binding = 3) uniform subpassInput GBuffer1;
layout (input_attachment_index = 2, set = 3, binding = 4) uniform subpassInput GBuffer2;
layout (input_attachment_index = 3, set = 3, binding = 5) uniform subpassInput DepthStencilBuffer;

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inViewRay;

layout (location = 0) out vec4 outFragColor;

struct GBufferVariables
{
	vec4 albedo_roughness;
	vec4 normal_ao;
	vec4 world_position;
	float metalic;
};

const vec3 lightPos = vec3(1000, 0, -1000);

vec3 ReconstructPosition()
{
	float window_z = subpassLoad(DepthStencilBuffer).r;
	float eye_z = (perFrameData.nearFar.x * perFrameData.nearFar.y) / (window_z * (perFrameData.nearFar.y - perFrameData.nearFar.x) - perFrameData.nearFar.y);

	vec3 viewRay = normalize(inViewRay);

	// Get cosine theta between view ray and camera direction
	float cos_viewRay_camDir = dot(viewRay, perFrameData.camDir.xyz);

	// "eye_z / cos_viewRay_camDir" is viewRay length between camera and surface point
	// NOTE: "eye_z" IS Z AXIS POSITION IN EYE SPACE, SO IT IS A MINUS VALUE, IF IT IS MULTIPLIED WITH A VECTOR, THEN WE SHOULD
	//       GET IT'S ABSOLUTE VALUE, I.E. DISTANCE TO CAMERA POSITION
	// DAMN THIS BUG HAUNTED ME FOR A DAY
	return viewRay * abs(eye_z) / cos_viewRay_camDir + perFrameData.camPos.xyz;
}


GBufferVariables UnpackGBuffers()
{
	GBufferVariables vars;

	vec4 gbuffer0 = subpassLoad(GBuffer0);
	vec4 gbuffer1 = subpassLoad(GBuffer1);
	vec4 gbuffer2 = subpassLoad(GBuffer2);

	vars.albedo_roughness.rgb = gbuffer1.rgb;
	vars.albedo_roughness.a = gbuffer2.r;

	vars.normal_ao.xyz = normalize(gbuffer0.xyz * 2.0f - 1.0f);
	vars.normal_ao.w = gbuffer2.a;

	vars.world_position = vec4(ReconstructPosition(), 1.0);

	vars.metalic = gbuffer2.g;

	return vars;
}

void main() 
{
	outFragColor = vec4(vec3(0), 1);

	GBufferVariables vars = UnpackGBuffers();

	vec3 n = vars.normal_ao.xyz;
	vec3 v = normalize(perFrameData.camPos.xyz - vars.world_position.xyz);
	vec3 l = normalize(lightPos - vars.world_position.xyz);
	vec3 h = normalize(l + v);

	float NdotH = max(0.0f, dot(n, h));
	float NdotL = max(0.0f, dot(n, l));
	float NdotV = max(0.0f, dot(n, v));
	float LdotH = max(0.0f, dot(l, h));

	F0 = mix(F0, vars.albedo_roughness.rgb, vars.metalic);

	vec3 fresnel = Fresnel_Schlick(F0, LdotH);
	vec3 kD = (1.0 - vars.metalic) * (vec3(1.0) - fresnel);

	//-------------- Ambient -----------------------
	vec3 fresnel_roughness = Fresnel_Schlick_Roughness(F0, NdotV, vars.albedo_roughness.a);
	vec3 kD_roughness = (1.0 - vars.metalic) * (vec3(1.0) - fresnel_roughness);

	vec3 irradiance = texture(RGBA16_512_CUBE_IRRADIANCE, vec3(n.x, -n.y, n.z)).rgb * vars.albedo_roughness.rgb / PI;

	vec3 reflectSampleDir = reflect(-v, n);
	reflectSampleDir.y *= -1.0;

	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = vars.albedo_roughness.a * MAX_REFLECTION_LOD;
	vec3 reflect = textureLod(RGBA16_512_CUBE_PREFILTERENV, reflectSampleDir, lod).rgb;
	vec2 brdf_lut = texture(RGBA16_512_2D_BRDFLUT, vec2(NdotV, vars.albedo_roughness.a)).rg;

	// Here we use NdotV rather than LdotH, since L's direction is based on punctual light, and here ambient reflection calculation
	// requires reflection vector dot with N, which is RdotN, equals NdotV
	vec3 ambient = (reflect * (brdf_lut.x * fresnel_roughness + brdf_lut.y) + irradiance * kD_roughness) * vars.normal_ao.a;
	//----------------------------------------------

	vec3 dirLightSpecular = fresnel * G_SchlicksmithGGX(NdotL, NdotV, vars.albedo_roughness.a) * GGX_D(NdotH, vars.albedo_roughness.a);
	vec3 dirLightDiffuse = vars.albedo_roughness.rgb * kD / PI;
	vec3 final = vars.normal_ao.a * ((dirLightSpecular + dirLightDiffuse) * NdotL * globalData.mainLightColor.rgb) + ambient;

	final = Uncharted2Tonemap(final * globalData.GEW.y);
	final = final * (1.0 / Uncharted2Tonemap(vec3(globalData.GEW.z)));
	final = pow(final, vec3(globalData.GEW.x));

	outFragColor = vec4(final, 1.0);
}