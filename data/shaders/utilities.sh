#if !defined(SHADER_UTILITIES)
#define SHADER_UTILITIES

#include "global_parameters.sh"

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

vec2 AcquireOneNearPosition(vec2 uv)
{
	vec2 corner = vec2(-1, 1);
	vec2 bottomLeft		= perFrameData.cameraSpaceSize.xy / 2.0f * corner.xx / perFrameData.nearFarAB.x;
	vec2 bottomRight	= perFrameData.cameraSpaceSize.xy / 2.0f * corner.yx / perFrameData.nearFarAB.x;
	vec2 topLeft		= perFrameData.cameraSpaceSize.xy / 2.0f * corner.xy / perFrameData.nearFarAB.x;
	vec2 topRight		= perFrameData.cameraSpaceSize.xy / 2.0f * corner.yy / perFrameData.nearFarAB.x;
	return mix(mix(topLeft, topRight, uv.x), mix(bottomLeft, bottomRight, uv.x), uv.y);
}

#endif