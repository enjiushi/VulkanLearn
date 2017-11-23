struct GlobalData
{
	// Camera Settings
	mat4 projection;
	mat4 vulkanNDC;
	mat4 PN;

	// Scene Settings
	vec4 mainLightDir;
	vec4 mainLightColor;

	// Render Settings
	vec4 GEW;		//Gamma, exposure, white scale
};

struct PerFrameData
{
	mat4 view;
	mat4 VPN;
	vec4 camPos;
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
	GlobalData globalData;
};

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