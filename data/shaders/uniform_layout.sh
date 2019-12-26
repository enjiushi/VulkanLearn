

struct GlobalData
{
	// Camera Settings
	mat4 projection;
	mat4 prevProj;

	// Window settings
	vec4 gameWindowSize;
	vec4 envGenWindowSize;
	vec4 shadowGenWindowSize;
	vec4 SSAOWindowSize;
	vec4 bloomWindowSize;
	vec4 motionTileWindowSize;

	// Scene Settings
	vec4 mainLightDir;
	vec4 mainLightColor;
	mat4 mainLightVP;

	// Main camera settings
	vec4 MainCameraSettings0;
	vec4 MainCameraSettings1;
	vec4 MainCameraSettings2;
	vec4 MainCameraSettings3;

	// Render Settings
	vec4 GEW;			//Gamma, exposure, white scale
	vec4 SSRSettings0;
	vec4 SSRSettings1;
	vec4 SSRSettings2;
	vec4 TemporalSettings0;
	vec4 BloomSettings0;
	vec4 BloomSettings1;
	vec4 DOFSettings0;
	vec4 MotionBlurSettings;
	vec4 VignetteSettings;
	vec4 SSAOSettings;
	vec4 SSAOSamples[64];
};

struct BoneData
{
	mat2x4 offsetDQ;
};

struct MeshData
{
	uint boneChunkIndexOffset;
};

struct AnimationData
{
	uint boneChunkIndexOffset;
};

struct PerFrameBoneData
{
	mat2x4 currAnimationDQ;
	mat2x4 prevAnimationDQ;
};

struct PerFrameData
{
	mat4 view;					
	mat4 viewCoordSystem;		
	mat4 prevView;			
	vec4 wsCameraPosition;
	vec4 wsCameraDeltaPosition;
	vec4 wsCameraDirection;
	vec4 cameraSpaceSize;
	vec4 nearFarAB;
	vec2 cameraJitterOffset;
	vec2 time;
	vec2 haltonX8Jitter;
	vec2 haltonX16Jitter;
	vec2 haltonX32Jitter;
	vec2 haltonX256Jitter;
	float frameIndex;
	float pingpongIndex;
	float reservedPadding0;
	float reservedPadding1;
};

struct PerObjectData
{
	mat4 MV;			// We can keep the translation of modelview matrix, as it's relative to camera. Larger number means far away, float rounding isn't visible
	mat4 MVP;			// Same as above
	mat4 MV_Rotation_P; // Model view matrix only has rotation, no translation

	mat4 prevMV;		// We can keep the translation of modelview matrix, as it's relative to camera. Larger number means far away, float rounding isn't visible
	mat4 prevMVP;
	mat4 prevMV_Rotation_P;
};

struct IndirectOffset
{
	int offset;
};

struct ObjectDataIndex
{
	int perObjectIndex;
	int perMaterialIndex;
	int perMeshIndex;
	int perAnimationIndex;
};

layout(set = 0, binding = 0) uniform GlobalUniforms
{
	GlobalData		globalData;
};

layout(set = 0, binding = 1) buffer PerBoneUniforms
{
	BoneData		boneData[];
};

layout(set = 0, binding = 2) buffer BoneIndirectUniforms
{
	uint	boneChunkIndirect[];
};

layout(set = 0, binding = 3) buffer PerFrameBoneIndirectUniforms
{
	uint	perFrameBoneChunkIndirect[];
};

layout(set = 0, binding = 4) buffer PerMeshUniforms
{
	MeshData		meshData[];
};

layout(set = 0, binding = 5) buffer PerAnimationUniforms
{
	AnimationData	animationData[];
};

layout(set = 0, binding = 6) uniform sampler2DArray RGBA8_1024_MIP_2DARRAY;
layout(set = 0, binding = 7) uniform sampler2DArray R8_1024_MIP_2DARRAY;
layout(set = 0, binding = 8) uniform sampler2DArray RGBA16_SCREEN_SIZE_MIP_2DARRAY;
layout(set = 0, binding = 9) uniform samplerCube RGBA16_1024_MIP_CUBE_SKYBOX;
layout(set = 0, binding = 10) uniform samplerCube RGBA16_512_CUBE_IRRADIANCE;
layout(set = 0, binding = 11) uniform samplerCube RGBA16_512_CUBE_PREFILTERENV;
layout(set = 0, binding = 12) uniform sampler2D RGBA16_512_2D_BRDFLUT;
layout(set = 0, binding = 13) uniform sampler2D SSAO_RANDOM_ROTATIONS;

layout(set = 1, binding = 0) uniform PerFrameUniforms
{
	PerFrameData perFrameData;
};

layout(set = 1, binding = 1) buffer PerFrameBoneUniforms
{
	PerFrameBoneData perFrameBoneData[];
};

layout(set = 2, binding = 0) buffer PerObjectUniforms
{
	PerObjectData perObjectData[];
};

layout(set = 3, binding = 1) buffer PerMaterialIndirectUniformOffset
{
	IndirectOffset indirectOffsets[];
};

layout(set = 3, binding = 2) buffer PerMaterialIndirectUniforms
{
	ObjectDataIndex objectDataIndex[];
};

int frameIndex = int(perFrameData.frameIndex);
int pingpongIndex = int(perFrameData.pingpongIndex);

const float PI = 3.1415926535897932384626433832795;
const float FLT_EPS = 0.00000001f;
vec3 F0 = vec3(0.04);
const vec3 up = { 0.0, 1.0, 0.0 };

float GGX_D(float NdotH, float roughness)
{
	float m2 = roughness * roughness;
	float m4 = m2 * m2;
	float f = NdotH * NdotH * (m4 - 1.0f) + 1.0f;
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

vec4 ImportanceSampleGGX(vec2 Xi, float Roughness)
{
	float m = Roughness * Roughness;
	float m2 = m * m;

	float Phi = 2 * PI * Xi.x;

	float CosTheta = sqrt((1.0 - Xi.y) / (1.0 + (m2 - 1.0) * Xi.y));
	float SinTheta = sqrt(max(1e-5, 1.0 - CosTheta * CosTheta));

	vec3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;

	float d = (CosTheta * m2 - CosTheta) * CosTheta + 1;
	float D = m2 / (PI * d * d);
	float pdf = D * CosTheta;

	return vec4(H, pdf);
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

const int sampleCount = 5;
const float weight[sampleCount] =
{
	0.227027,
	0.1945946,
	0.1216216,
	0.054054,
	0.016216
};

vec4 Blur(sampler2D tex, vec2 uv, int direction, float scale, float strength)
{
	vec2 step = vec2(1.0f) / textureSize(tex, 0).xy;

	vec2 dir = vec2(0.0f, 1.0f);
	if (direction == 1)
		dir = vec2(1.0f, 0.0f);

	dir = dir * step;

	vec4 result = texture(tex, uv).rgba * weight[0];
	for (int i = 1; i < sampleCount; i++)
	{
		result += texture(tex, uv + dir * i * scale).rgba * weight[i];
		result += texture(tex, uv + dir * -i * scale).rgba * weight[i];
	}

	return result * strength;
}

// Copied from unity shader pack
// Better, temporally stable box filtering
// [Jimenez14] http://goo.gl/eomGso
// . . . . . . .
// . A . B . C .
// . . D . E . .
// . F . G . H .
// . . I . J . .
// . K . L . M .
// . . . . . . .
vec4 DownsampleBox13Tap(sampler2D tex, vec2 uv, vec2 texelSize)
{
	vec4 A = texture(tex, (uv + texelSize * vec2(-1.0, -1.0)));
	vec4 B = texture(tex, (uv + texelSize * vec2(0.0, -1.0)));
	vec4 C = texture(tex, (uv + texelSize * vec2(1.0, -1.0)));
	vec4 D = texture(tex, (uv + texelSize * vec2(-0.5, -0.5)));
	vec4 E = texture(tex, (uv + texelSize * vec2(0.5, -0.5)));
	vec4 F = texture(tex, (uv + texelSize * vec2(-1.0, 0.0)));
	vec4 G = texture(tex, (uv));
	vec4 H = texture(tex, (uv + texelSize * vec2(1.0, 0.0)));
	vec4 I = texture(tex, (uv + texelSize * vec2(-0.5, 0.5)));
	vec4 J = texture(tex, (uv + texelSize * vec2(0.5, 0.5)));
	vec4 K = texture(tex, (uv + texelSize * vec2(-1.0, 1.0)));
	vec4 L = texture(tex, (uv + texelSize * vec2(0.0, 1.0)));
	vec4 M = texture(tex, (uv + texelSize * vec2(1.0, 1.0)));

	vec2 div = (1.0 / 4.0) * vec2(0.5, 0.125);

	vec4 o = (D + E + I + J) * div.x;
	o += (A + B + G + F) * div.y;
	o += (B + C + H + G) * div.y;
	o += (F + G + L + K) * div.y;
	o += (G + H + M + L) * div.y;

	return o;
}

// Copied from unity shader pack
// 9-tap bilinear upsampler (tent filter)
vec4 UpsampleTent(sampler2D tex, vec2 uv, vec2 texelSize, vec4 sampleScale)
{
	vec4 d = texelSize.xyxy * vec4(1.0, 1.0, -1.0, 0.0) * sampleScale;

	vec4 s;
	s = texture(tex, (uv - d.xy));
	s += texture(tex, (uv - d.wy)) * 2.0;
	s += texture(tex, (uv - d.zy));

	s += texture(tex, (uv + d.zw)) * 2.0;
	s += texture(tex, (uv)) * 4.0;
	s += texture(tex, (uv + d.xw)) * 2.0;

	s += texture(tex, (uv + d.zy));
	s += texture(tex, (uv + d.wy)) * 2.0;
	s += texture(tex, (uv + d.xy));

	return s * (1.0 / 16.0);
}

float PDnrand(vec2 n) 
{
	return fract(sin(dot(n.xy, vec2(12.9898f, 78.233f)))* 43758.5453f);
}

vec2 PDnrand2(vec2 n)
{
	return fract(sin(dot(n.xy, vec2(12.9898f, 78.233f)))* vec2(43758.5453f, 28001.8384f));
}

vec3 PDnrand3(vec2 n)
{
	return fract(sin(dot(n.xy, vec2(12.9898f, 78.233f)))* vec3(43758.5453f, 28001.8384f, 50849.4141f));
}

vec4 PDnrand4(vec2 n)
{
	return fract(sin(dot(n.xy, vec2(12.9898f, 78.233f)))* vec4(43758.5453f, 28001.8384f, 50849.4141f, 12996.89f));
}


float PDsrand(vec2 n) 
{
	return PDnrand(n) * 2 - 1;
}
vec2 PDsrand2(vec2 n)
{
	return PDnrand2(n) * 2 - 1;
}

vec3 PDsrand3(vec2 n)
{
	return PDnrand3(n) * 2 - 1;
}

vec4 PDsrand4(vec2 n)
{
	return PDnrand4(n) * 2 - 1;
}

struct GBufferVariables
{
	vec4 albedo_roughness;
	vec4 normal_ao;
	vec4 camera_position;
	float metalic;
	float shadowFactor;
	float ssaoFactor;
};

float AcquireShadowFactor(vec4 csPosition, sampler2D ShadowMapDepthBuffer)
{
	// The view matrix in main light VP needs to be the transfrom from main camera space rather than world space
	// Doing this to avoid large number of world space position in a large scale scene
	vec4 lsPosition = globalData.mainLightVP * csPosition;
	lsPosition /= lsPosition.w;
	lsPosition.xy = lsPosition.xy * 0.5f + 0.5f;	// NOTE: Don't do this to z, as it's already within [0, 1] after vulkan ndc transform

	lsPosition.z = max(0, lsPosition.z);

	vec2 texelSize = 1.0f / textureSize(ShadowMapDepthBuffer, 0);
	float shadowFactor = 0.0f;
	float pcfDepth;
	float bias = -0.0005f;

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

	vars.albedo_roughness.rgb = gbuffer1.rgb;
	vars.albedo_roughness.a = gbuffer2.r;

	vars.normal_ao.xyz = normalize(gbuffer0.xyz * 2.0f - 1.0f);
	vars.normal_ao.w = gbuffer2.a;

	float linearDepth;
	vars.camera_position = vec4(ReconstructCSPosition(coord, oneNearPosition, DepthStencilBuffer, linearDepth), 1.0f);

	vars.metalic = gbuffer2.g;

	return vars;
}

GBufferVariables UnpackGBuffers(ivec2 coord, vec2 texcoord, vec2 oneNearPosition, sampler2D GBuffer0, sampler2D GBuffer1, sampler2D GBuffer2, sampler2D DepthStencilBuffer, sampler2D BlurredSSAOBuffer, sampler2D ShadowMapDepthBuffer)
{
	GBufferVariables vars;

	vec4 gbuffer0 = texelFetch(GBuffer0, coord, 0);
	vec4 gbuffer1 = texelFetch(GBuffer1, coord, 0);
	vec4 gbuffer2 = texelFetch(GBuffer2, coord, 0);

	vars.albedo_roughness.rgb = gbuffer1.rgb;
	vars.albedo_roughness.a = gbuffer2.r;

	vars.normal_ao.xyz = gbuffer0.xyz * 2.0f - 1.0f;
	vars.normal_ao.w = gbuffer2.a;

	float linearDepth;
	vars.camera_position = vec4(ReconstructCSPosition(coord, oneNearPosition, DepthStencilBuffer, linearDepth), 1.0);

	vars.metalic = gbuffer2.g;

	vars.shadowFactor = AcquireShadowFactor(vars.camera_position, ShadowMapDepthBuffer);

	vars.ssaoFactor = texture(BlurredSSAOBuffer, texcoord).r;

	vars.ssaoFactor = min(1.0f, vars.ssaoFactor);
    vars.ssaoFactor = min(1.0f, pow(vars.ssaoFactor, globalData.SSAOSettings.w));

	return vars;
}

float Luminance(in vec3 color)
{
	return dot(color, vec3(0.2126f, 0.7152f, 0.0722f));
}

vec3 clip_aabb(vec3 aabb_min, vec3 aabb_max, vec3 p)
{
	// note: only clips towards aabb center (but fast!)
	vec3 p_clip = 0.5f * (aabb_max + aabb_min);
	vec3 e_clip = 0.5f * (aabb_max - aabb_min) + FLT_EPS;

	vec3 v_clip = p - p_clip;
	vec3 v_unit = v_clip / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if (ma_unit > 1.0f)
		return p_clip + v_clip / ma_unit;
	else
		return p;// point inside aabb
}

float CalculateCoC(float eyeDepth)
{
	float coc = (eyeDepth - globalData.MainCameraSettings1.x) * globalData.DOFSettings0.z / max(eyeDepth, 1e-5);
	return clamp(coc * 0.5f * globalData.DOFSettings0.y + 0.5f, 0, 1);
}

int GetIndirectIndex(int drawID, int instanceID)
{
	return indirectOffsets[drawID].offset + instanceID;
}

