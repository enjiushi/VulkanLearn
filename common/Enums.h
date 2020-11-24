#pragma once

enum CubeFace
{
	RIGHT,
	LEFT,
	TOP,
	BOTTOM,
	FRONT,
	BACK,
	CUBE_FACE_COUNT
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