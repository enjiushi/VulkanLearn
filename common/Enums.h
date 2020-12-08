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

enum class TileAdjacency
{
	TOP_LEFT,
	TOP,
	TOP_RIGHT,
	MIDDLE_LEFT,
	MIDDLE,
	MIDDLE_RIGHT,
	BOTTOM_LEFT,
	BOTTOM,
	BOTTOM_RIGHT,
	COUNT
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