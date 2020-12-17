#pragma once

enum class CubeFace
{
	RIGHT,
	LEFT,
	TOP,
	BOTTOM,
	FRONT,
	BACK,
	COUNT
};

enum class CubeVertex
{
	BOTTOM_LEFT_FRONT,
	BOTTOM_RIGHT_FRONT,
	TOP_LEFT_FRONT,
	TOP_RIGHT_FRONT,
	BOTTOM_LEFT_BACK,
	BOTTOM_RIGHT_BACK,
	TOP_LEFT_BACK,
	TOP_RIGHT_BACK,
	COUNT
};

enum class Axis
{
	X,
	Y,
	Z,
	COUNT,
};

enum class NormCoordAxis
{
	U,
	V,
	W,
	COUNT,
};

enum class Sign
{
	POSITIVE = 1,
	NEGATIVE = -1,
	COUNT
};

enum class TileAdjacency
{
	BOTTOM_LEFT,
	BOTTOM,
	BOTTOM_RIGHT,
	LEFT,
	MIDDLE,
	RIGHT,
	TOP_LEFT,
	TOP,
	TOP_RIGHT,
	COUNT
};

enum class AdjacentTileOffsetU
{
	LEFT = -1,
	MIDDLE = 0,
	RIGHT = 1,
	COUNT = 3
};

enum class AdjacentTileOffsetV
{
	BOTTOM = -1,
	MIDDLE = 0,
	TOP = 1,
	COUNT = 3
};

enum VertexAttribFlag
{
	VAFPosition,
	VAFNormal,
	VAFColor,
	VAFTexCoord,
	VAFTangent,
	VAFBone,
	VACount
};

enum VertexFormat
{
	VertexFormatNul = 0,
	VertexFormatP = (1 << VAFPosition),
	VertexFormatPN = (1 << VAFPosition) | (1 << VAFNormal),
	VertexFormatPTC = (1 << VAFPosition) | (1 << VAFTexCoord),
	VertexFormatPNTC = (1 << VAFPosition) | (1 << VAFNormal) | (1 << VAFTexCoord),
	VertexFormatPNTCT = (1 << VAFPosition) | (1 << VAFNormal) | (1 << VAFTexCoord) | (1 << VAFTangent),
	VertexFormatPNTCTB = (1 << VAFPosition) | (1 << VAFNormal) | (1 << VAFTexCoord) | (1 << VAFTangent) | (1 << VAFBone)
};

// Reserved vertex buffer binding slot, don't use these slot
enum ReservedVBBindingSlot
{
	ReservedVBBindingSlot_MeshData,
	// Maybe more in future
	ReservedVBBindingSlotCount
};

enum class CullState
{
	CULL,			// If a triangle is fully out of a volumn
	CULL_DIVIDE,	// If a triangle intersects with a volumn
	DIVIDE			// If a triangle is fully inside a volumn
};

extern Axis CubeFaceAxisMapping[(uint32_t)CubeFace::COUNT][(uint32_t)NormCoordAxis::COUNT];