#if !defined(SHADER_UNIFORM_LAYOUT)
#define SHADER_UNIFORM_LAYOUT

#extension GL_EXT_scalar_block_layout :enable

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
	vec4 PlanetRenderingSettings0;
	vec4 PlanetRenderingSettings1;
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

struct PlanetData
{
	vec4	planetDescriptor0;
	float	PlanetLODDistanceLUT[32];
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
	int utilityIndex;	// Could be used to indirect to material specific data, like bone, or planet
};

layout(std430, set = 0, binding = 0) uniform GlobalUniforms
{
	GlobalData		globalData;
};

layout(std430, set = 0, binding = 1) buffer PerBoneUniforms
{
	BoneData		boneData[];
};

layout(std430, set = 0, binding = 2) buffer BoneIndirectUniforms
{
	uint	boneChunkIndirect[];
};

layout(std430, set = 0, binding = 3) buffer PerFrameBoneIndirectUniforms
{
	uint	perFrameBoneChunkIndirect[];
};

layout(std430, set = 0, binding = 4) buffer PerMeshUniforms
{
	MeshData		meshData[];
};

layout(std430, set = 0, binding = 5) buffer PerPlanetUniforms
{
	PlanetData		planetData[];
};

layout(std430, set = 0, binding = 6) buffer PerAnimationUniforms
{
	AnimationData	animationData[];
};

layout(set = 0, binding = 7) uniform sampler2DArray RGBA8_1024_MIP_2DARRAY;
layout(set = 0, binding = 8) uniform sampler2DArray R8_1024_MIP_2DARRAY;
layout(set = 0, binding = 9) uniform sampler2DArray RGBA16_SCREEN_SIZE_MIP_2DARRAY;
layout(set = 0, binding = 10) uniform samplerCube RGBA16_1024_MIP_CUBE_SKYBOX;
layout(set = 0, binding = 11) uniform samplerCube RGBA16_512_CUBE_IRRADIANCE;
layout(set = 0, binding = 12) uniform samplerCube RGBA16_512_CUBE_PREFILTERENV;
layout(set = 0, binding = 13) uniform sampler2D RGBA16_512_2D_BRDFLUT;
layout(set = 0, binding = 14) uniform sampler2D SSAO_RANDOM_ROTATIONS;

layout(std430, set = 1, binding = 0) uniform PerFrameUniforms
{
	PerFrameData perFrameData;
};

layout(std430, set = 1, binding = 1) buffer PerFrameBoneUniforms
{
	PerFrameBoneData perFrameBoneData[];
};

layout(std430, set = 2, binding = 0) buffer PerObjectUniforms
{
	PerObjectData perObjectData[];
};

layout(std430, set = 3, binding = 1) buffer PerMaterialIndirectUniformOffset
{
	IndirectOffset indirectOffsets[];
};

layout(std430, set = 3, binding = 2) buffer PerMaterialIndirectUniforms
{
	ObjectDataIndex objectDataIndex[];
};

#endif