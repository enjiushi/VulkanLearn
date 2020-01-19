#if !defined(SHADER_GBUFFER_RECONSTRUCTION)
#define SHADER_GBUFFER_RECONSTRUCTION

struct GBufferVariables
{
	vec4 albedoRoughness;
	vec4 normalAO;
	vec4 csPosition;
	float metalic;
	float shadowFactor;
	float ssaoFactor;
};

float ReconstructLinearDepth(float sampledDepth)
{
	return -1.0f * perFrameData.nearFarAB.x / sampledDepth;
}

vec3 ReconstructCSPosition(in ivec2 coord, in vec3 CSViewRay, in sampler2D DepthBuffer, out float linearDepth)
{
	float window_z = texelFetch(DepthBuffer, coord, 0).r;
	linearDepth = ReconstructLinearDepth(window_z);

	// Get cosine theta between view ray and camera direction
	float cos_viewRay_camDir = dot(CSViewRay, vec3(0, 0, -1));

	// abs(linearDepth) / cos_viewRay_camDir is length of view ray in camera space
	return CSViewRay * abs(linearDepth) / cos_viewRay_camDir;
}

vec3 ReconstructCSPosition(in ivec2 coord, in vec2 oneNearPosition, in sampler2D DepthBuffer, out float linearDepth)
{
	float window_z = texelFetch(DepthBuffer, coord, 0).r;
	linearDepth = ReconstructLinearDepth(window_z);

	// Let interpolated position multiply with camera space linear depth to reconstruct camera space position
	return vec3(oneNearPosition * abs(linearDepth), linearDepth);
}

float AcquireShadowFactor(vec4 csPosition, sampler2D ShadowMapDepthBuffer)
{
	// The view matrix in main light VP needs to be the transfrom from main camera space rather than world space
	// Doing this to avoid large number of world space position in a large scale scene
	vec4 lsPosition = perFrameData.mainLightVP * csPosition;
	lsPosition /= lsPosition.w;
	lsPosition.xy = lsPosition.xy * 0.5f + 0.5f;	// NOTE: Don't do this to z, as it's already within [0, 1] after vulkan ndc transform

	lsPosition.z = max(0, lsPosition.z);

	vec2 texelSize = 1.0f / textureSize(ShadowMapDepthBuffer, 0);
	float shadowFactor = 0.0f;
	float pcfDepth;
	float bias = 0.0f;

	pcfDepth = texture(ShadowMapDepthBuffer, lsPosition.xy + vec2(-1, -1) * texelSize).r + bias;
	shadowFactor += (lsPosition.z < pcfDepth ? 1.0 : 0.0) * 0.077847;

	pcfDepth = texture(ShadowMapDepthBuffer, lsPosition.xy + vec2(0, -1) * texelSize).r + bias;
	shadowFactor += (lsPosition.z < pcfDepth ? 1.0 : 0.0) * 0.123317;

	pcfDepth = texture(ShadowMapDepthBuffer, lsPosition.xy + vec2(1, -1) * texelSize).r + bias;
	shadowFactor += (lsPosition.z < pcfDepth ? 1.0 : 0.0) * 0.077847;

	pcfDepth = texture(ShadowMapDepthBuffer, lsPosition.xy + vec2(-1, 0) * texelSize).r + bias;
	shadowFactor += (lsPosition.z < pcfDepth ? 1.0 : 0.0) * 0.123317;

	pcfDepth = texture(ShadowMapDepthBuffer, lsPosition.xy + vec2(0, 0) * texelSize).r + bias;
	shadowFactor += (lsPosition.z < pcfDepth ? 1.0 : 0.0) * 0.195346;

	pcfDepth = texture(ShadowMapDepthBuffer, lsPosition.xy + vec2(1, 0) * texelSize).r + bias;
	shadowFactor += (lsPosition.z < pcfDepth ? 1.0 : 0.0) * 0.123317;

	pcfDepth = texture(ShadowMapDepthBuffer, lsPosition.xy + vec2(-1, 1) * texelSize).r + bias;
	shadowFactor += (lsPosition.z < pcfDepth ? 1.0 : 0.0) * 0.077847;

	pcfDepth = texture(ShadowMapDepthBuffer, lsPosition.xy + vec2(0, 1) * texelSize).r + bias;
	shadowFactor += (lsPosition.z < pcfDepth ? 1.0 : 0.0) * 0.123317;

	pcfDepth = texture(ShadowMapDepthBuffer, lsPosition.xy + vec2(1, 1) * texelSize).r + bias;
	shadowFactor += (lsPosition.z < pcfDepth ? 1.0 : 0.0) * 0.077847;

	return 1.0f - shadowFactor;
}

GBufferVariables UnpackGBuffers(ivec2 coord, vec2 oneNearPosition, sampler2D GBuffer0, sampler2D GBuffer1, sampler2D GBuffer2, sampler2D DepthStencilBuffer)
{
	GBufferVariables vars;

	vec4 gbuffer0 = texelFetch(GBuffer0, coord, 0);
	vec4 gbuffer1 = texelFetch(GBuffer1, coord, 0);
	vec4 gbuffer2 = texelFetch(GBuffer2, coord, 0);

	vars.albedoRoughness.rgb = gbuffer1.rgb;
	vars.albedoRoughness.a = gbuffer2.r;

	vars.normalAO.xyz = normalize(gbuffer0.xyz * 2.0f - 1.0f);
	vars.normalAO.w = gbuffer2.a;

	float linearDepth;
	vars.csPosition = vec4(ReconstructCSPosition(coord, oneNearPosition, DepthStencilBuffer, linearDepth), 1.0f);

	vars.metalic = gbuffer2.g;

	return vars;
}

GBufferVariables UnpackGBuffers(ivec2 coord, vec2 texcoord, vec2 oneNearPosition, sampler2D GBuffer0, sampler2D GBuffer1, sampler2D GBuffer2, sampler2D DepthStencilBuffer, sampler2D BlurredSSAOBuffer, sampler2D ShadowMapDepthBuffer)
{
	GBufferVariables vars;

	vec4 gbuffer0 = texelFetch(GBuffer0, coord, 0);
	vec4 gbuffer1 = texelFetch(GBuffer1, coord, 0);
	vec4 gbuffer2 = texelFetch(GBuffer2, coord, 0);

	vars.albedoRoughness.rgb = gbuffer1.rgb;
	vars.albedoRoughness.a = gbuffer2.r;

	vars.normalAO.xyz = gbuffer0.xyz * 2.0f - 1.0f;
	vars.normalAO.w = gbuffer2.a;

	float linearDepth;
	vars.csPosition = vec4(ReconstructCSPosition(coord, oneNearPosition, DepthStencilBuffer, linearDepth), 1.0);

	vars.metalic = gbuffer2.g;

	vars.shadowFactor = AcquireShadowFactor(vars.csPosition, ShadowMapDepthBuffer);

	vars.ssaoFactor = texture(BlurredSSAOBuffer, texcoord).r;

	vars.ssaoFactor = min(1.0f, vars.ssaoFactor);
    vars.ssaoFactor = min(1.0f, pow(vars.ssaoFactor, globalData.SSAOSettings.w));

	return vars;
}

#endif