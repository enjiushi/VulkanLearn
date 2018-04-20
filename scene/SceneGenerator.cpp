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

	m_pMesh0 = GenerateQuadMesh();
	m_pMaterial0 = GenerateBRDFLUTGenMaterial(m_pMesh0);
	m_pMaterialInstance0 = m_pMaterial0->CreateMaterialInstance();
	m_pMaterialInstance0->SetRenderMask(1 << RenderWorkManager::BrdfLutGen);
	m_pMeshRenderer0 = MeshRenderer::Create(m_pMesh0, { m_pMaterialInstance0 });

	std::shared_ptr<BaseObject> pQuadObj = BaseObject::Create();
	pQuadObj->AddComponent(m_pMeshRenderer0);
	m_pRootObj->AddChild(pQuadObj);

	StagingBufferMgr()->FlushDataMainThread();
}

std::shared_ptr<Mesh> SceneGenerator::GenerateBoxMesh()
{
	// FIXME: Put this into utility classes
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
		cubeVertices, 8, 1 << VAFPosition,
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
		cubeVertices, 24, (1 << VAFPosition) | (1 << VAFNormal) | (1 << VAFTexCoord) | (1 << VAFTangent),
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
		quadVertices, 4, (1 << VAFPosition) | (1 << VAFNormal) | (1 << VAFTexCoord) | (1 << VAFTangent),
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
		quadVertices, 4, (1 << VAFPosition) | (1 << VAFTexCoord),
		quadIndices, 6, VK_INDEX_TYPE_UINT32
	);
}


std::shared_ptr<ForwardMaterial> SceneGenerator::GenerateIrradianceGenMaterial(const std::shared_ptr<Mesh>& pMesh)
{
	SimpleMaterialCreateInfo info = {};
	info.shaderPaths = { L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/irradiance.frag.spv", L"" };
	info.vertexBindingsInfo = { pMesh->GetVertexBuffer()->GetBindingDesc() };
	info.vertexAttributesInfo = pMesh->GetVertexBuffer()->GetAttribDesc();
	info.materialUniformVars = {};
	info.vertexFormat = pMesh->GetVertexBuffer()->GetVertexFormat();
	info.subpassIndex = 0;
	info.pRenderPass = RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen();
	info.frameBufferType = FrameBufferDiction::FrameBufferType_EnvGenOffScreen;

	return ForwardMaterial::CreateDefaultMaterial(info);
}

std::shared_ptr<ForwardMaterial> SceneGenerator::GeneratePrefilterEnvGenMaterial(const std::shared_ptr<Mesh>& pMesh)
{
	SimpleMaterialCreateInfo info = {};
	info.shaderPaths = { L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/prefilter_env.frag.spv", L"" };
	info.vertexBindingsInfo = { pMesh->GetVertexBuffer()->GetBindingDesc() };
	info.vertexAttributesInfo = pMesh->GetVertexBuffer()->GetAttribDesc();
	info.materialUniformVars = {};
	info.vertexFormat = pMesh->GetVertexBuffer()->GetVertexFormat();
	info.subpassIndex = 0;
	info.pRenderPass = RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen();
	info.frameBufferType = FrameBufferDiction::FrameBufferType_EnvGenOffScreen;

	return ForwardMaterial::CreateDefaultMaterial(info);
}

std::shared_ptr<ForwardMaterial> SceneGenerator::GenerateBRDFLUTGenMaterial(const std::shared_ptr<Mesh>& pMesh)
{
	SimpleMaterialCreateInfo info = {};
	info.shaderPaths = { L"../data/shaders/brdf_lut.vert.spv", L"", L"", L"", L"../data/shaders/brdf_lut.frag.spv", L"" };
	info.vertexBindingsInfo = { pMesh->GetVertexBuffer()->GetBindingDesc() };
	info.vertexAttributesInfo = pMesh->GetVertexBuffer()->GetAttribDesc();
	info.materialUniformVars = {};
	info.vertexFormat = pMesh->GetVertexBuffer()->GetVertexFormat();
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