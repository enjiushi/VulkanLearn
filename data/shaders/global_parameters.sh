#if !defined(SHADER_GLOBAL_PARAMETERS)
#define SHADER_GLOBAL_PARAMETERS

#include "uniform_layout.sh"

int frameIndex = int(perFrameData.frameIndex);
int pingpongIndex = int(perFrameData.pingpongIndex);

#if !defined(PI_DEFINED)
#define PI_DEFINED
const float PI = 3.1415926535897932384626433832795;
#endif

const float FLT_EPS = 0.00000001f;
vec3 F0 = vec3(0.04);
const vec3 up = { 0.0, 1.0, 0.0 };

const int sampleCount = 5;
const float weight[sampleCount] =
{
	0.227027,
	0.1945946,
	0.1216216,
	0.054054,
	0.016216
};

#endif