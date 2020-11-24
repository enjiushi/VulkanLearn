#include "../vulkan/SharedVertexBuffer.h"
#include "../vulkan/StagingBufferManager.h"
#include "../class/RenderWorkManager.h"
#include "../class/Mesh.h"
#include "../class/Material.h"
#include "../class/MaterialInstance.h"
#include "../class/GlobalTextures.h"
#include "../component/MeshRenderer.h"
#include "../component/Camera.h"
#include "../class/RenderPassDiction.h"
#include "SceneGenerator.h"
#include "../class/ForwardRenderPass.h"
#include "../class/ForwardMaterial.h"
#include "../class/RenderPassDiction.h"
#include "../class/FrameBufferDiction.h"
#include "../common/Util.h"
#include "../common/Enums.h"

void SceneGenerator::PurgeExcistSceneData()
{
	m_pRootObj = nullptr;
	m_pCameraObj = nullptr;

	m_pMesh0 = nullptr;
	m_pMeshRenderer0 = nullptr;

	m_pMaterialInstance0 = nullptr;
	m_pMaterial0 = nullptr;
}

void SceneGenerator::GenerateBRDFLUTGenScene()
{
	PurgeExcistSceneData();

	m_pRootObj = BaseObject::Create();

	m_pCameraObj = GenerateIBLGenOffScreenCamera((uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x);
	m_pRootObj->AddChild(m_pCameraObj);

	m_pMaterial0 = GenerateBRDFLUTGenMaterial();
	m_pMaterialInstance0 = m_pMaterial0->CreateMaterialInstance();
	m_pMaterialInstance0->SetRenderMask(1 << RenderWorkManager::BrdfLutGen);
	m_pMeshRenderer0 = MeshRenderer::Create(nullptr, { m_pMaterialInstance0 });

	std::shared_ptr<BaseObject> pQuadObj = BaseObject::Create();
	pQuadObj->AddComponent(m_pMeshRenderer0);
	m_pRootObj->AddChild(pQuadObj);

	StagingBufferMgr()->FlushDataMainThread();
}

void SceneGenerator::GenerateCube(Vector3d vertices[], uint32_t indices[])
{
	vertices[0] = { -1, -1,  1 };
	vertices[1] = {  1, -1,  1 };
	vertices[2] = { -1, -1, -1 };
	vertices[3] = {  1, -1, -1 };

	vertices[4] = { -1,  1,  1 };
	vertices[5] = {  1,  1,  1 };
	vertices[6] = { -1,  1, -1 };
	vertices[7] = {  1,  1, -1 };

	for (uint32_t i = 0; i < 8; i++)
		vertices[i].Normalize();

	// Right
	indices[0] = 1;
	indices[1] = 3;
	indices[2] = 5;
	indices[3] = 5;
	indices[4] = 3;
	indices[5] = 7;

	// Left
	indices[6] = 2;
	indices[7] = 0;
	indices[8] = 6;
	indices[9] = 6;
	indices[10] = 0;
	indices[11] = 4;

	// Top
	indices[12] = 4;
	indices[13] = 5;
	indices[14] = 6;
	indices[15] = 6;
	indices[16] = 5;
	indices[17] = 7;

	// Bottom
	indices[18] = 1;
	indices[19] = 0;
	indices[20] = 3;
	indices[21] = 3;
	indices[22] = 0;
	indices[23] = 2;

	// Front
	indices[24] = 0;
	indices[25] = 1;
	indices[26] = 4;
	indices[27] = 4;
	indices[28] = 1;
	indices[29] = 5;

	// Back
	indices[30] = 3;
	indices[31] = 2;
	indices[32] = 7;
	indices[33] = 7;
	indices[34] = 2;
	indices[35] = 6;
}

std::shared_ptr<Mesh> SceneGenerator::GenerateLODTriangleMesh(uint32_t level, bool forQuad)
{
	std::vector<Vector4f> vertices;
	std::vector<uint32_t> indices;

	uint32_t rowCount = (uint32_t)std::pow(2, level) + 1;
	float subdivideLength = 1 / (float)(rowCount - 1);
	uint32_t lastRowstartIndex = 0;
	uint32_t currentRowStartIndex = 0;
	for (uint32_t row = 0; row < rowCount; row++)
	{
		if (row == 0)
		{
			vertices.push_back({ 0, 0, 0, 0 });
			lastRowstartIndex = 0;
			currentRowStartIndex = 1;
			continue;
		}

		double ratioStep = (1.0 / (double)row);
		for (uint32_t index = 0; index < (row + 1); index++)
		{
			double edgeLength = row * subdivideLength;
			vertices.push_back({ (float)(edgeLength * (1.0 - index * ratioStep)), (float)(edgeLength * (index * ratioStep)), 0, 0 });
		}

		for (uint32_t evenIndex = 0; evenIndex < row; evenIndex++)
		{
			indices.push_back(lastRowstartIndex + evenIndex);
			indices.push_back(currentRowStartIndex + evenIndex);
			indices.push_back(currentRowStartIndex + evenIndex + 1);
		}

		for (uint32_t oddIndex = 0; oddIndex < row - 1; oddIndex++)
		{
			indices.push_back(currentRowStartIndex + 1 + oddIndex);
			indices.push_back(lastRowstartIndex + oddIndex + 1);
			indices.push_back(lastRowstartIndex + oddIndex);
		}

		lastRowstartIndex = currentRowStartIndex;
		currentRowStartIndex = (uint32_t)vertices.size();
	}

	// Add morph factors for each vertex

	if (!forQuad)
	{
		for (uint32_t row = 0; row < rowCount; row++)
		{
			currentRowStartIndex = row * (row + 1) / 2;

			bool flag = (row % 2 != 0);

			if (flag)
			{
				for (uint32_t i = 0; i < row + 1; i++)
				{
					if (i % 2 == 0)
					{
						vertices[currentRowStartIndex + i].z = vertices[currentRowStartIndex + i].x + subdivideLength;
						vertices[currentRowStartIndex + i].w = vertices[currentRowStartIndex + i].y;
					}
					else
					{
						vertices[currentRowStartIndex + i].z = vertices[currentRowStartIndex + i].x;
						vertices[currentRowStartIndex + i].w = vertices[currentRowStartIndex + i].y - subdivideLength;
					}
				}
			}
			else
			{
				for (uint32_t i = 0; i < row + 1; i++)
				{
					vertices[currentRowStartIndex + i].z = vertices[currentRowStartIndex + i].x;
					vertices[currentRowStartIndex + i].w = vertices[currentRowStartIndex + i].y;
				}

				uint32_t index = currentRowStartIndex;

				for (uint32_t i = 0; i < row / 2; i++)
				{
					vertices[index + 1].z = vertices[index + 1].x - subdivideLength;
					vertices[index + 1].w = vertices[index + 1].y + subdivideLength;

					index += 2;
				}
			}
		}
	}
	else
	{
		for (uint32_t row = 0; row < rowCount; row++)
		{
			currentRowStartIndex = row * (row + 1) / 2;

			bool flag = (row % 2 != 0);

			if (flag)
			{
				for (uint32_t i = 0; i < row + 1; i++)
				{
					if (i % 2 == 0)
					{
						vertices[currentRowStartIndex + i].z = vertices[currentRowStartIndex + i].x + subdivideLength;
						vertices[currentRowStartIndex + i].w = vertices[currentRowStartIndex + i].y;
					}
					else
					{
						vertices[currentRowStartIndex + i].z = vertices[currentRowStartIndex + i].x;
						vertices[currentRowStartIndex + i].w = vertices[currentRowStartIndex + i].y + subdivideLength;
					}
				}
			}
			else
			{
				for (uint32_t i = 0; i < row + 1; i++)
				{
					vertices[currentRowStartIndex + i].z = vertices[currentRowStartIndex + i].x;
					vertices[currentRowStartIndex + i].w = vertices[currentRowStartIndex + i].y;
				}

				uint32_t index = currentRowStartIndex;

				for (uint32_t i = 0; i < row / 2; i++)
				{
					vertices[index + 1].z = vertices[index + 1].x - subdivideLength;
					vertices[index + 1].w = vertices[index + 1].y + subdivideLength;

					index += 2;
				}
			}
		}
	}

	std::shared_ptr<Mesh> pTriangleMesh = Mesh::Create
	(
		vertices.data(), (uint32_t)vertices.size(), 1 << VAFColor,
		indices.data(), (uint32_t)indices.size(), VK_INDEX_TYPE_UINT32
	);

	return pTriangleMesh;
}

std::shared_ptr<Mesh> SceneGenerator::GenerateLODQuadMesh(uint32_t level)
{
	std::vector<Vector4f> vertices;
	std::vector<uint32_t> indices;

	uint32_t divideCount = (uint32_t)std::pow(2, level);
	float subdivideLength = 1 / (float)divideCount;

	Vector2f morphStartPosition;
	Vector2f morphEndPosition;

	for (uint32_t row = 0; row < divideCount + 1; row++)
	{
		for (uint32_t column = 0; column < divideCount + 1; column++)
		{
			morphEndPosition = { subdivideLength * column, subdivideLength * row };
			morphStartPosition = morphEndPosition;

			if (row % 2 != 0)
				morphStartPosition.y += subdivideLength;
			if (column % 2 != 0)
				morphStartPosition.x += subdivideLength;

			vertices.push_back({ morphEndPosition.x, morphEndPosition.y, morphStartPosition.x, morphStartPosition.y });
		}
	}

	for (uint32_t row = 0; row < divideCount; row++)
	{
		for (uint32_t column = 0; column < divideCount; column++)
		{
			indices.push_back((row + 1) * (divideCount + 1) + column);
			indices.push_back((row + 1 )* (divideCount + 1) + column + 1);
			indices.push_back(row * (divideCount + 1) + column);

			indices.push_back(row * (divideCount + 1) + column);
			indices.push_back((row + 1)* (divideCount + 1) + column + 1);
			indices.push_back(row* (divideCount + 1) + column + 1);
		}
	}

	return Mesh::Create
	(
		vertices.data(), (uint32_t)vertices.size(), 1 << VAFColor,
		indices.data(), (uint32_t)indices.size(), VK_INDEX_TYPE_UINT32
	);
}

std::shared_ptr<Mesh> SceneGenerator::GenerateBoxMesh()
{
	float cubeVertices[] = {
		// front
		-1.0, -1.0,  1.0,
		1.0, -1.0,  1.0,
		1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		// back
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0,
	};

	uint32_t cubeIndices[] = {
		// front
		0, 2, 1,
		2, 0, 3,
		// top
		1, 6, 5,
		6, 1, 2,
		// back
		7, 5, 6,
		5, 7, 4,
		// bottom
		4, 3, 0,
		3, 4, 7,
		// left
		4, 1, 5,
		1, 4, 0,
		// right
		3, 6, 2,
		6, 3, 7,
	};

	std::shared_ptr<Mesh> pCubeMesh = Mesh::Create
	(
		cubeVertices, 8, VertexFormatP,
		cubeIndices, 36, VK_INDEX_TYPE_UINT32
	);

	return pCubeMesh;
}

std::shared_ptr<Mesh> SceneGenerator::GeneratePBRBoxMesh()
{
	// FIXME: Put this into utility classes
	float cubeVertices[] = {
		// front
		-1.0, -1.0,  1.0,  0.0, 0.0, 1.0,  0.0, 0.0,  1.0, 0.0, 0.0,
		1.0, -1.0,  1.0,  0.0, 0.0, 1.0,  1.0, 0.0,  1.0, 0.0, 0.0,
		1.0,  1.0,  1.0,  0.0, 0.0, 1.0,  1.0, 1.0,  1.0, 0.0, 0.0,
		-1.0,  1.0,  1.0,  0.0, 0.0, 1.0,  0.0, 1.0,  1.0, 0.0, 0.0,

		// back
		-1.0, -1.0, -1.0,  0.0, 0.0, -1.0,  0.0, 0.0,  -1.0, 0.0, 0.0,
		1.0, -1.0, -1.0,  0.0, 0.0, -1.0,  1.0, 0.0,  -1.0, 0.0, 0.0,
		1.0,  1.0, -1.0,  0.0, 0.0, -1.0,  1.0, 1.0,  -1.0, 0.0, 0.0,
		-1.0,  1.0, -1.0,  0.0, 0.0, -1.0,  0.0, 1.0,  -1.0, 0.0, 0.0,

		// top
		-1.0, 1.0,  -1.0,  0.0, 1.0, 0.0,  0.0, 0.0,  1.0, 0.0, 0.0,
		1.0, 1.0,  -1.0,  0.0, 1.0, 0.0,  1.0, 0.0,  1.0, 0.0, 0.0,
		1.0,  1.0,  1.0,  0.0, 1.0, 0.0,  1.0, 1.0,  1.0, 0.0, 0.0,
		-1.0,  1.0,  1.0,  0.0, 1.0, 0.0,  0.0, 1.0,  1.0, 0.0, 0.0,

		// bottom
		-1.0, -1.0,  -1.0,  0.0, -1.0, 0.0,  0.0, 0.0,  -1.0, 0.0, 0.0,
		1.0, -1.0,  -1.0,  0.0, -1.0, 0.0,  1.0, 0.0,  -1.0, 0.0, 0.0,
		1.0,  -1.0,  1.0,  0.0, -1.0, 0.0,  1.0, 1.0,  -1.0, 0.0, 0.0,
		-1.0,  -1.0,  1.0,  0.0, -1.0, 0.0,  0.0, 1.0,  -1.0, 0.0, 0.0,

		// left
		-1.0, -1.0,  -1.0,  -1.0, 0.0, 0.0,  0.0, 0.0,  0.0, 1.0, 0.0,
		-1.0, -1.0,  1.0,  -1.0, 0.0, 0.0,  1.0, 0.0,  0.0, 1.0, 0.0,
		-1.0,  1.0,  1.0,  -1.0, 0.0, 0.0,  1.0, 1.0,  0.0, 1.0, 0.0,
		-1.0,  1.0,  -1.0,  -1.0, 0.0, 0.0,  0.0, 1.0,  0.0, 1.0, 0.0,

		// right
		1.0, -1.0,  -1.0,  1.0, 0.0, 0.0,  0.0, 0.0,  0.0, -1.0, 0.0,
		1.0, -1.0,  1.0,  1.0, 0.0, 0.0,  1.0, 0.0,  0.0, -1.0, 0.0,
		1.0,  1.0,  1.0,  1.0, 0.0, 0.0,  1.0, 1.0,  0.0, -1.0, 0.0,
		1.0,  1.0,  -1.0,  1.0, 0.0, 0.0,  0.0, 1.0,  0.0, -1.0, 0.0,
	};

	uint32_t cubeIndices[] = {
		// front
		0, 1, 2,
		0, 2, 3,

		// back
		4, 6, 5,
		4, 7, 6,

		// top
		8, 10, 9,
		8, 11, 10,

		// bottom
		12, 13, 14,
		12, 14, 15,

		// left
		16, 17, 18,
		16, 18, 19,

		// right
		20, 22, 21,
		20, 23, 22,
	};

	return Mesh::Create
	(
		cubeVertices, 24, VertexFormatPNTCT,
		cubeIndices, 36, VK_INDEX_TYPE_UINT32
	);
}

std::shared_ptr<Mesh> SceneGenerator::GeneratePBRQuadMesh()
{
	float quadVertices[] = {
		-1.0, -1.0,  0.0,   0.0, 0.0, 1.0,   0.0, 0.0,   1.0, 0.0, 0.0,
		1.0, -1.0,  0.0,   0.0, 0.0, 1.0,   1.0, 0.0,   1.0, 0.0, 0.0,
		-1.0,  1.0,  0.0,   0.0, 0.0, 1.0,   0.0, 1.0,   1.0, 0.0, 0.0,
		1.0,  1.0,  0.0,   0.0, 0.0, 1.0,   1.0, 1.0,   1.0, 0.0, 0.0,
	};

	uint32_t quadIndices[] = {
		0, 1, 3,
		0, 3, 2,
	};

	return Mesh::Create
	(
		quadVertices, 4, VertexFormatPNTCT,
		quadIndices, 6, VK_INDEX_TYPE_UINT32
	);
}

std::shared_ptr<Mesh> SceneGenerator::GenerateQuadMesh()
{
	float quadVertices[] = {
		-1.0, -1.0,  0.0, 0.0, 0.0, 0.0,
		1.0, -1.0,  0.0,  1.0, 0.0, 0.0,
		-1.0,  1.0,  0.0, 0.0, 1.0, 0.0,
		1.0,  1.0,  0.0,  1.0, 1.0, 0.0,
	};

	uint32_t quadIndices[] = {
		0, 1, 3,
		0, 3, 2,
	};

	return Mesh::Create
	(
		quadVertices, 4, VertexFormatPTC,
		quadIndices, 6, VK_INDEX_TYPE_UINT32
	);
}

// FIXME: code for testing, will remove later
std::shared_ptr<Mesh> SceneGenerator::GenPBRIcosahedronMesh()
{
	uint32_t divideLevel = 1;

	float ratio = (1.0f + sqrt(5.0f)) / 2.0f;
	float scale = 1.0f / glm::length(glm::vec2(ratio, 1.0f));
	ratio *= scale;

	Vector3d icoVertices[] = 
	{
		{ ratio, 0, -scale },			//rf 0
		{ -ratio, 0, -scale },			//lf 1
		{ ratio, 0, scale },			//rb 2
		{ -ratio, 0, scale },			//lb 3
												 
		{ 0, -scale, ratio },			//db 4
		{ 0, -scale, -ratio },			//df 5
		{ 0, scale, ratio },			//ub 6
		{ 0, scale, -ratio },			//uf 7
												 
		{ -scale, ratio, 0 },			//lu 8
		{ -scale, -ratio, 0 },			//ld 9
		{ scale, ratio, 0 },			//ru 10
		{ scale, -ratio, 0 }			//rd 11
	};

	uint32_t indices[20 * 3] =
	{
		1, 3, 8,
		3, 1, 9,
		0, 10, 2,
		2, 11, 0,

		5, 7, 0,
		7, 5, 1,
		4, 2, 6,
		6, 3, 4,

		9, 11, 4,
		11, 9, 5,
		8, 6, 10,
		10, 7, 8,

		1, 8, 7,
		5, 9, 1,
		0, 7, 10,
		5, 0, 11,

		3, 6, 8,
		4, 3, 9,
		2, 10, 6,
		4, 11, 2
	};

	return Mesh::Create
	(
		&icoVertices[0], 12, VertexFormatP,
		&indices[0], 20 * 3, VK_INDEX_TYPE_UINT32
	);
}

std::shared_ptr<ForwardMaterial> SceneGenerator::GenerateBRDFLUTGenMaterial()
{
	SimpleMaterialCreateInfo info = {};
	info.shaderPaths = { L"../data/shaders/brdf_lut.vert.spv", L"", L"", L"", L"../data/shaders/brdf_lut.frag.spv", L"" };
	info.materialUniformVars = {};
	info.subpassIndex = 0;
	info.pRenderPass = RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen();
	info.frameBufferType = FrameBufferDiction::FrameBufferType_EnvGenOffScreen;

	return ForwardMaterial::CreateDefaultMaterial(info);
}

std::shared_ptr<BaseObject> SceneGenerator::GenerateIBLGenOffScreenCamera(uint32_t screenSize)
{
	CameraInfo camInfo =
	{
		3.1415 / 2.0,
		1.0,
		1.0,
		2000.0,
	};
	std::shared_ptr<BaseObject> pOffScreenCamObj = BaseObject::Create();
	std::shared_ptr<Camera> pOffScreenCamComp = Camera::Create(camInfo);
	pOffScreenCamObj->AddComponent(pOffScreenCamComp);

	return pOffScreenCamObj;
}