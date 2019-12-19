#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

layout (set = 3, binding = 3) uniform sampler2D GBuffer0[3];
layout (set = 3, binding = 4) uniform sampler2D GBuffer1[3];
layout (set = 3, binding = 5) uniform sampler2D GBuffer2[3];
layout (set = 3, binding = 6) uniform sampler2D MotionVector[3];
layout (set = 3, binding = 7) uniform sampler2D DepthStencilBuffer[3];
layout (set = 3, binding = 8) uniform sampler2D ShadowMapDepthBuffer[3];
layout (set = 3, binding = 9) uniform sampler2D BlurredSSAOBuffer[3];
layout (set = 3, binding = 10) uniform sampler2D SSRInfo[3];

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec2 inOneNearPosition;

layout (location = 0) out vec4 outShadingColor;
layout (location = 1) out vec4 outSSRColor;

float rayTraceMaxStep = globalData.SSRSettings1.z;
float rayTraceBorderFadeDist = globalData.SSRSettings2.x;
float rayTraceMaxStepFadeDist = globalData.SSRSettings2.y;
float screenSizeMiplevel = globalData.SSRSettings2.z;

const vec2 offset[4] =
{
	vec2(0, 0),
	vec2(2, -2),
	vec2(-2, -2),
	vec2(0, 2)
};


float BorderFading(vec2 hitUV)
{
	float borderDist = min(1.0 - max(hitUV.x, hitUV.y), min(hitUV.x, hitUV.y));
	//return 1.0f - pow(borderDist, 10.1f);
	return smoothstep(0.0f, rayTraceBorderFadeDist, borderDist);
}

vec4 CalculateSSR(vec3 n, vec3 v, float NdotV, vec4 albedoRoughness, vec3 CSPosition, float metalic, vec3 skyBoxReflection)
{
	ivec2 coord = ivec2(inUv * globalData.gameWindowSize.xy);

	vec4 SSRRadiance = vec4(0);
	float weightSum = 0.0f;

	vec2 rand2 = normalize(PDsrand2(inUv + vec2(perFrameData.time.x)));
	mat2 offsetRotation = mat2(rand2.x, rand2.y, -rand2.y, rand2.x);

	float coneTangent = mix(0.0, albedoRoughness.a * (1.0f - globalData.SSRSettings0.x), NdotV * sqrt(albedoRoughness.a));

	int count = 4;
	for (int i = 0; i < count; i++)
	{
		vec4 SSRHitInfo = texelFetch(SSRInfo[frameIndex], (coord + ivec2(offsetRotation * offset[i])) / 2, 0);
		float hitFlag = sign(SSRHitInfo.a) * 0.5f + 0.5f;
		vec2 hitUV = SSRHitInfo.xy * globalData.gameWindowSize.zw;

		vec2 motionVec = texelFetch(MotionVector[frameIndex], ivec2(SSRHitInfo.xy + perFrameData.cameraJitterOffset * globalData.gameWindowSize.xy), 0).rg;

		float intersectionCircleRadius = coneTangent * length(hitUV - inUv);
		float mip = clamp(log2(intersectionCircleRadius * max(globalData.gameWindowSize.x, globalData.gameWindowSize.y)), 0.0, screenSizeMiplevel) * globalData.SSRSettings0.y;

		vec3 SSRSurfColor = textureLod(RGBA16_SCREEN_SIZE_MIP_2DARRAY, vec3(hitUV + motionVec, 0), mip).rgb * hitFlag;

		float SSRSurfDepth;
		vec3 SSRSurfPosition = ReconstructCSPosition(ivec2(SSRHitInfo.xy), inOneNearPosition, DepthStencilBuffer[frameIndex], SSRSurfDepth);

		vec3 l = normalize(SSRSurfPosition.xyz - CSPosition);
		vec3 h = normalize(l + v);

		float NdotH = max(0.0f, dot(n, h));
		float NdotL = max(0.0f, dot(n, l));
		float LdotH = max(0.0f, dot(l, h));

		float weight = 1.0f;
		if (hitFlag < 0.5f)
			SSRSurfColor = skyBoxReflection;

		// Reenable this when I figure how to deal with none-hit pixels
		//if (hitFlag > 0.5f)
			//weight = GGX_D(NdotH, albedoRoughness.a) * G_SchlicksmithGGX(NdotL, NdotV, albedoRoughness.a) / (4.0f * NdotL * NdotV + 0.001f) / max(SSRHitInfo.b, 1e-5);
		//else
		//{
			//weight = 0.5f;	// FIXME?
			//SSRSurfColor = vec3(0);
		//}
		weight = max(weight, 0.001f);

		SSRSurfColor /= 1.0f + Luminance(SSRSurfColor);

		SSRRadiance.rgb += SSRSurfColor * weight;
		//SSRRadiance.a += hitFlag * weight;
		SSRRadiance.a += hitFlag * weight * (1.0f - smoothstep(1.0f - rayTraceMaxStepFadeDist, 1.0f, abs(SSRHitInfo.a) / rayTraceMaxStep)) * BorderFading(hitUV);

		weightSum += weight;
	}

	SSRRadiance /= weightSum;	// normalize

	return SSRRadiance;
}

void main() 
{
	ivec2 coord = ivec2(floor(inUv * globalData.gameWindowSize.xy));

	GBufferVariables vars = UnpackGBuffers(coord, inUv, inOneNearPosition, GBuffer0[frameIndex], GBuffer1[frameIndex], GBuffer2[frameIndex], DepthStencilBuffer[frameIndex], BlurredSSAOBuffer[frameIndex], ShadowMapDepthBuffer[frameIndex]);

	if (length(vars.normal_ao.xyz) > 1.1f)
		discard;

	vec3 n = normalize(vars.normal_ao.xyz);
	vec3 v = normalize(-vec3(inOneNearPosition, -1));	// Beware, you'll have to multiply camera space linear depth -1(or any depth you want), and view ray goes towards camera
	vec3 l = globalData.mainLightDir.xyz;
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

	vec3 reflectSampleDir = mat3(perFrameData.viewCoordSystem) * reflect(-v, n);
	reflectSampleDir.y *= -1.0;

	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = vars.albedo_roughness.a * MAX_REFLECTION_LOD;
	vec3 reflect = textureLod(RGBA16_512_CUBE_PREFILTERENV, reflectSampleDir, lod).rgb;

	vec2 brdf_lut = texture(RGBA16_512_2D_BRDFLUT, vec2(NdotV, vars.albedo_roughness.a)).rg;

	// Here we use NdotV rather than LdotH, since L's direction is based on punctual light, and here ambient reflection calculation
	// requires reflection vector dot with N, which is RdotN, equals NdotV
	vec4 SSRRadiance = CalculateSSR(n, v, NdotV, vars.albedo_roughness, vars.camera_position.xyz, vars.metalic, reflect);

	float aoFactor = min(vars.normal_ao.a, 1.0f - vars.ssaoFactor);

	vec3 skyBoxRadiance = mix(reflect, SSRRadiance.rgb, SSRRadiance.a);
	vec3 brdfFactor = brdf_lut.x * fresnel_roughness + brdf_lut.y;

	skyBoxRadiance *= brdfFactor;
	irradiance *= kD_roughness;

	vec3 skyBoxAmbient	= (irradiance + skyBoxRadiance) * aoFactor;
	//----------------------------------------------

	vec3 dirLightSpecular = fresnel * G_SchlicksmithGGX(NdotL, NdotV, vars.albedo_roughness.a) * min(1.0f, GGX_D(NdotH, vars.albedo_roughness.a)) / (4.0f * NdotL * NdotV + 0.001f);
	vec3 dirLightDiffuse = vars.albedo_roughness.rgb * kD / PI;
	vec3 punctualRadiance = vars.shadowFactor * ((dirLightSpecular + dirLightDiffuse) * NdotL * globalData.mainLightColor.rgb);

	outShadingColor = vec4(punctualRadiance, vars.albedo_roughness.a);
	outSSRColor = vec4(skyBoxAmbient, SSRRadiance.a);
}