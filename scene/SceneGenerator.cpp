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

void SceneGenerator::GenerateIrradianceGenScene()
{
	PurgeExcistSceneData();

	m_pRootObj = BaseObject::Create();

	m_pCameraObj = GenerateIBLGenOffScreenCamera(UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x);
	m_pRootObj->AddChild(m_pCameraObj);

	m_pMesh0 = GenerateBoxMesh();
	m_pMaterial0 = GenerateIrradianceGenMaterial(m_pMesh0);
	m_pMaterialInstance0 = m_pMaterial0->CreateMaterialInstance();
	m_pMaterialInstance0->SetRenderMask(1 << RenderWorkManager::IrradianceGen);
	m_pMeshRenderer0 = MeshRenderer::Create(m_pMesh0, { m_pMaterialInstance0 });

	std::shared_ptr<BaseObject> pSkyBox = BaseObject::Create();
	pSkyBox->AddComponent(m_pMeshRenderer0);
	m_pRootObj->AddChild(pSkyBox);

	StagingBufferMgr()->FlushDataMainThread();
}

void SceneGenerator::GeneratePrefilterEnvGenScene()
{
	PurgeExcistSceneData();

	m_pRootObj = BaseObject::Create();

	m_pCameraObj = GenerateIBLGenOffScreenCamera(UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x);
	m_pRootObj->AddChild(m_pCameraObj);

	m_pMesh0 = GenerateBoxMesh();
	m_pMaterial0 = GeneratePrefilterEnvGenMaterial(m_pMesh0);
	m_pMaterialInstance0 = m_pMaterial0->CreateMaterialInstance();
	m_pMaterialInstance0->SetRenderMask(1 << RenderWorkManager::ReflectionGen);
	m_pMeshRenderer0 = MeshRenderer::Create(m_pMesh0, { m_pMaterialInstance0 });

	std::shared_ptr<BaseObject> pSkyBox = BaseObject::Create();
	pSkyBox->AddComponent(m_pMeshRenderer0);
	m_pRootObj->AddChild(pSkyBox);

	StagingBufferMgr()->FlushDataMainThread();
}

void SceneGenerator::GenerateBRDFLUTGenScene()
{
	PurgeExcistSceneData();

	m_pRootObj = BaseObject::Create();

	m_pCameraObj = GenerateIBLGenOffScreenCamera(UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x);
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

std::shared_ptr<Mesh> SceneGenerator::GenerateTriangleMesh()
{
	float triangleVertices[] = {
		-1.0, -1.0,  0.5,
		1.0, -1.0,  0.5,
		0.0,  1.0,  0.5,
	};

	uint32_t cubeIndices[] = {
		0, 1, 2
	};

	std::shared_ptr<Mesh> pTriangleMesh = Mesh::Create
	(
		triangleVertices, 3, VertexFormatP,
		cubeIndices, 3, VK_INDEX_TYPE_UINT32
	);

	return pTriangleMesh;
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

	Vector3f icoVertices[] = 
	{
		{ ratio, 0, -scale },			//rf 0
		{ -ratio, 0, -scale },		//lf 1
		{ ratio, 0, scale },			//rb 2
		{ -ratio, 0, scale },			//lb 3
												 
		{ 0, -scale, ratio },			//db 4
		{ 0, -scale, -ratio },		//df 5
		{ 0, scale, ratio },			//ub 6
		{ 0, scale, -ratio },			//uf 7
												 
		{ -scale, ratio, 0 },			//lu 8
		{ -scale, -ratio, 0 },		//ld 9
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

std::shared_ptr<ForwardMaterial> SceneGenerator::GenerateIrradianceGenMaterial(const std::shared_ptr<Mesh>& pMesh)
{
	SimpleMaterialCreateInfo info = {};
	info.shaderPaths = { L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/irradiance.frag.spv", L"" };
	info.materialUniformVars = {};
	info.vertexFormat = pMesh->GetVertexBuffer()->GetVertexFormat();
	info.vertexFormatInMem = pMesh->GetVertexBuffer()->GetVertexFormat();
	info.subpassIndex = 0;
	info.pRenderPass = RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen();
	info.frameBufferType = FrameBufferDiction::FrameBufferType_EnvGenOffScreen;

	return ForwardMaterial::CreateDefaultMaterial(info);
}

std::shared_ptr<ForwardMaterial> SceneGenerator::GeneratePrefilterEnvGenMaterial(const std::shared_ptr<Mesh>& pMesh)
{
	SimpleMaterialCreateInfo info = {};
	info.shaderPaths = { L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/prefilter_env.frag.spv", L"" };
	info.materialUniformVars = {};
	info.vertexFormat = pMesh->GetVertexBuffer()->GetVertexFormat();
	info.vertexFormatInMem = pMesh->GetVertexBuffer()->GetVertexFormat();
	info.subpassIndex = 0;
	info.pRenderPass = RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen();
	info.frameBufferType = FrameBufferDiction::FrameBufferType_EnvGenOffScreen;

	return ForwardMaterial::CreateDefaultMaterial(info);
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
		3.1415f / 2.0f,
		screenSize / screenSize,
		1.0f,
		2000.0f,
	};
	std::shared_ptr<BaseObject> pOffScreenCamObj = BaseObject::Create();
	std::shared_ptr<Camera> pOffScreenCamComp = Camera::Create(camInfo);
	pOffScreenCamObj->AddComponent(pOffScreenCamComp);

	return pOffScreenCamObj;
}