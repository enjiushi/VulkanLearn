#include "VulkanGlobal.h"
#include "../common/Macros.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include <array>
#include "../maths/Matrix.h"
#include <math.h>
#include "Importer.hpp"
#include "scene.h"
#include "postprocess.h"
#include "Buffer.h"
#include "StagingBuffer.h"
#include "Queue.h"
#include "StagingBufferManager.h"
#include "FrameManager.h"
#include "../thread/ThreadWorker.hpp"
#include <gli\gli.hpp>
#include "SharedVertexBuffer.h"
#include "SharedIndexBuffer.h"
#include "../class/RenderWorkManager.h"
#include <iostream>
#include "GlobalVulkanStates.h"
#include "../class/UniformData.h"
#include "IndirectBuffer.h"
#include "SharedIndirectBuffer.h"
#include "../class/Enums.h"
#include "../class/GlobalTextures.h"
#include "../scene/SceneGenerator.h"
#include "../class/RenderPassDiction.h"
#include "../class/ForwardRenderPass.h"
#include "../class/DeferredRenderPass.h"

void VulkanGlobal::InitVulkanInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "VulkanLearn";
	appInfo.apiVersion = (((1) << 22) | ((0) << 12) | (0));

	//Need surface extension to create surface from device
	std::vector<const char*> extensions = { EXTENSION_VULKAN_SURFACE };
	std::vector<const char*> layers;
#if defined(_WIN32)
	extensions.push_back(EXTENSION_VULKAN_SURFACE_WIN32);
#endif
#if defined(_DEBUG)
	layers.push_back(EXTENSION_VULKAN_VALIDATION_LAYER);
	extensions.push_back(EXTENSION_VULKAN_DEBUG_REPORT);
#endif
	VkInstanceCreateInfo instCreateInfo = {};
	instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instCreateInfo.pApplicationInfo = &appInfo;
	instCreateInfo.enabledExtensionCount = (int32_t)extensions.size();
	instCreateInfo.ppEnabledExtensionNames = extensions.data();
	instCreateInfo.enabledLayerCount = (int32_t)layers.size();
	instCreateInfo.ppEnabledLayerNames = layers.data();

	m_pVulkanInstance = Instance::Create(instCreateInfo);
	assert(m_pVulkanInstance != nullptr);
}

void VulkanGlobal::InitPhysicalDevice(HINSTANCE hInstance, HWND hWnd)
{
	m_pPhysicalDevice = PhysicalDevice::Create(m_pVulkanInstance, hInstance, hWnd);
	ASSERTION(m_pPhysicalDevice != nullptr);
}

void VulkanGlobal::InitVulkanDevice()
{
	m_pDevice = Device::Create(m_pVulkanInstance, m_pPhysicalDevice);
	ASSERTION(m_pDevice != nullptr);
}

#if defined (_WIN32)
void VulkanGlobal::SetupWindow(HINSTANCE hinstance, WNDPROC wndproc)
{
	m_hPlatformInst = hinstance;

	bool fullscreen = false;

	// Check command line arguments
	for (int32_t i = 0; i < __argc; i++)
	{
		if (__argv[i] == std::string("-fullscreen"))
		{
			fullscreen = true;
		}
	}

	WNDCLASSEX wndClass;

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = wndproc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hinstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = PROJECT_NAME;
	wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClass))
	{
		std::cout << "Could not register window class!\n";
		fflush(stdout);
		exit(1);
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = screenWidth;
		dmScreenSettings.dmPelsHeight = screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if ((WINDOW_WIDTH != screenWidth) && (WINDOW_HEIGHT != screenHeight))
		{
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					fullscreen = FALSE;
				}
				else
				{
					return;
				}
			}
		}

	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRect;
	if (fullscreen)
	{
		windowRect.left = (long)0;
		windowRect.right = (long)screenWidth;
		windowRect.top = (long)0;
		windowRect.bottom = (long)screenHeight;
	}
	else
	{
		windowRect.left = (long)screenWidth / 2 - WINDOW_WIDTH / 2;
		windowRect.right = (long)WINDOW_WIDTH;
		windowRect.top = (long)screenHeight / 2 - WINDOW_HEIGHT / 2;
		windowRect.bottom = (long)WINDOW_HEIGHT;
	}

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	std::string windowTitle = PROJECT_NAME;
	m_hWindow = CreateWindowEx(0,
		PROJECT_NAME,
		windowTitle.c_str(),
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		windowRect.left,
		windowRect.top,
		windowRect.right,
		windowRect.bottom,
		NULL,
		NULL,
		hinstance,
		NULL);

	if (!m_hWindow)
	{
		printf("Could not create window!\n");
		fflush(stdout);
		exit(1);
	}

	ShowWindow(m_hWindow, SW_SHOW);
	SetForegroundWindow(m_hWindow);
	SetFocus(m_hWindow);
}

#if defined(_WIN32)
#define KEY_ESCAPE VK_ESCAPE 
#define KEY_F1 VK_F1
#define KEY_F2 VK_F2
#define KEY_F3 VK_F3
#define KEY_F4 VK_F4
#define KEY_F5 VK_F5
#define KEY_W 0x57
#define KEY_A 0x41
#define KEY_S 0x53
#define KEY_D 0x44
#define KEY_P 0x50
#define KEY_SPACE 0x20
#define KEY_KPADD 0x6B
#define KEY_KPSUB 0x6D
#define KEY_B 0x42
#define KEY_F 0x46
#define KEY_L 0x4C
#define KEY_N 0x4E
#define KEY_O 0x4F
#define KEY_T 0x54
#endif

void VulkanGlobal::HandleMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	float x, y, width, height;
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case KEY_ESCAPE:
			PostQuitMessage(0);
			break;
		}

		switch (wParam)
		{
		case KEY_W:
			m_moveFlag |= CharMoveDir::Forward;
			break;
		case KEY_S:
			m_moveFlag |= CharMoveDir::Backward;
			break;
		case KEY_A:
			m_moveFlag |= CharMoveDir::Leftward;
			break;
		case KEY_D:
			m_moveFlag |= CharMoveDir::Rightward;
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case KEY_W:
			m_moveFlag &= ~(CharMoveDir::Forward);
			break;
		case KEY_S:
			m_moveFlag &= ~(CharMoveDir::Backward);
			break;
		case KEY_A:
			m_moveFlag &= ~(CharMoveDir::Leftward);
			break;
		case KEY_D:
			m_moveFlag &= ~(CharMoveDir::Rightward);
			break;
		}
		break;
	case WM_RBUTTONDOWN:
		x = (float)LOWORD(lParam);
		y = (float)HIWORD(lParam);
		width = GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
		height = GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
		m_pCharacter->OnRotate({ x / width, (height - y) / height }, true);
		break;
	case WM_RBUTTONUP:
		x = (float)LOWORD(lParam);
		y = (float)HIWORD(lParam);
		width = GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
		height = GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
		m_pCharacter->OnRotate({ x / width, (height - y) / height }, false);
		break;
	case WM_MOUSEMOVE:
		if (wParam & MK_RBUTTON)
		{
			x = (float)LOWORD(lParam);
			y = (float)HIWORD(lParam);
			width = GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
			height = GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
			m_pCharacter->OnRotate({ x / width, (height - y) / height }, true);
		}
		break;
	}
}

#endif

void VulkanGlobal::InitQueue()
{
}

void VulkanGlobal::InitSurface()
{
}

void VulkanGlobal::InitSwapchain()
{
}

void VulkanGlobal::Update()
{
#if defined(_WIN32)
	static uint32_t frameCount = 0;
	static float fpsTimer = 0;
	MSG msg;
	bool quitMessageReceived = false;
	while (!quitMessageReceived)
	{
		auto tStart = std::chrono::high_resolution_clock::now();
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				quitMessageReceived = true;
				FrameMgr()->WaitForAllJobsDone();
				return;
			}
		}
		Draw();
		frameCount++;
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

		fpsTimer += (float)tDiff;
		if (fpsTimer > 1000.0f)
		{
			std::stringstream ss;
			ss << "FPS:" << frameCount;
			SetWindowText(m_hWindow, ss.str().c_str());
			fpsTimer = 0.0f;
			frameCount = 0.0f;
		}
	}
#endif
}

void VulkanGlobal::InitCommandPool()
{
	m_pCommandPool = CommandPool::Create(m_pDevice);
}

void VulkanGlobal::InitSetupCommandBuffer()
{
}

void VulkanGlobal::InitMemoryMgr()
{
}

void VulkanGlobal::InitSwapchainImgs()
{
}

void VulkanGlobal::InitDepthStencil()
{
	m_pDSBuffer = DepthStencilBuffer::Create(m_pDevice);
}

void VulkanGlobal::InitRenderpass()
{
}

void VulkanGlobal::InitFrameBuffer()
{
}

void VulkanGlobal::InitVertices()
{
	m_pGunMesh = Mesh::Create("../data/textures/cerberus/cerberus.fbx");
	m_pSphereMesh = Mesh::Create("../data/models/sphere.obj");

	VkVertexInputBindingDescription bindingDesc = {};
	std::vector<VkVertexInputAttributeDescription> attribDesc;

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

	m_pCubeMesh = Mesh::Create
	(
		cubeVertices, 8, 1 << VAFPosition,
		cubeIndices, 36, VK_INDEX_TYPE_UINT32
	);

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

	m_pQuadMesh = Mesh::Create
	(
		quadVertices, 4, (1 << VAFPosition) | (1 << VAFTexCoord), 
		quadIndices, 6, VK_INDEX_TYPE_UINT32
	);
}

// Replace rgbTex's alpha channel with rTex's red channel
// This utility function is used only here to achieve texture packing as a preparation for later texture array implementation
void CombineRGBA8_R8_RGBA8(gli::texture2d& rgbaTex, gli::texture2d rTex)
{

	ASSERTION(rgbaTex.extent().x == rTex.extent().x && rgbaTex.extent().y == rTex.extent().y);
	ASSERTION(rgbaTex.layers() == rTex.layers() && rgbaTex.levels() == rTex.levels());

	std::vector<uint8_t> buffer;
	buffer.resize(rgbaTex.size());

	uint8_t* rgbaTexData = (uint8_t*)rgbaTex.data();
	uint8_t* rTexData = (uint8_t*)rTex.data();

	for (uint32_t i = 0; i < rTex.size(); i++)
	{
		rgbaTexData[i * 4 + 3] = rTexData[i];
	}
}

void VulkanGlobal::InitUniforms()
{
	gli::texture2d gliAlbedoTex(gli::load("../data/textures/cerberus/albedo_1024.ktx"));
	gli::texture2d gliRoughnessTex(gli::load("../data/textures/cerberus/roughness_1024.ktx"));
	CombineRGBA8_R8_RGBA8(gliAlbedoTex, gliRoughnessTex);

	gli::texture2d gliNormalTex(gli::load("../data/textures/cerberus/normal_1024.ktx"));
	gli::texture2d gliAOTex(gli::load("../data/textures/cerberus/ao_1024.ktx"));
	CombineRGBA8_R8_RGBA8(gliNormalTex, gliAOTex);

	gli::texture2d gliMetalic(gli::load("../data/textures/cerberus/metallic_1024.ktx"));

	m_pAlbedoRoughness = Texture2D::Create(m_pDevice, { {gliAlbedoTex} }, VK_FORMAT_R8G8B8A8_UNORM);
	m_pMetalic = Texture2D::Create(m_pDevice, { {gliMetalic} }, VK_FORMAT_R8_UNORM);
	m_pNormalAO = Texture2D::Create(m_pDevice, { {gliNormalTex} }, VK_FORMAT_R8G8B8A8_UNORM);

	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "GunAlbedoRoughness", "", "RGB:Albedo, A:Roughness" }, gliAlbedoTex);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "GunNormalAO", "", "RGB:Normal, A:AO" }, gliNormalTex);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::R8_1024, { "GunMetallic", "", "R:Metalic" }, gliMetalic);

	gli::texture_cube gliSkyBox(gli::load("../data/textures/hdr/gcanyon_cube.ktx"));
	UniformData::GetInstance()->GetGlobalTextures()->InitIBLTextures(gliSkyBox);
	//UniformData::GetInstance()->GetGlobalUniforms()->GetGlobalTextures()->GetIBLTextureCube(IBLTextureType::RGBA16_1024_SkyBox)->UpdateByteStream({ { gliSkyBox } });
	
	
	m_pSkyBoxTex = TextureCube::Create(m_pDevice, "../data/textures/hdr/gcanyon_cube.ktx", VK_FORMAT_R16G16B16A16_SFLOAT);
	
	//m_pSimpleTex = Texture2D::Create(m_pDevice, "../data/textures/cerberus/albedo.ktx", VK_FORMAT_R8G8B8A8_UNORM);
	m_pSimpleTex = Texture2D::CreateEmptyTexture(m_pDevice, OffScreenSize, OffScreenSize, VK_FORMAT_R16G16B16A16_SFLOAT);
	m_pIrradianceTex = TextureCube::CreateEmptyTextureCube(m_pDevice, OffScreenSize, OffScreenSize, VK_FORMAT_R16G16B16A16_SFLOAT);
	m_pPrefilterEnvTex = TextureCube::CreateEmptyTextureCube(m_pDevice, OffScreenSize, OffScreenSize, std::log2(OffScreenSize) + 1, VK_FORMAT_R16G16B16A16_SFLOAT);

	// FIXME: 2 channels should be enough, I don't intend to do it now as I have to create new render passes and frame buffers, leave it to later refactor
	m_pBRDFLut = Texture2D::CreateEmptyTexture(m_pDevice, OffScreenSize, OffScreenSize, VK_FORMAT_R16G16B16A16_SFLOAT);
}

void VulkanGlobal::InitDescriptorSetLayout()
{
}

void VulkanGlobal::InitPipelineCache()
{
	/*
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	CHECK_VK_ERROR(vkCreatePipelineCache(m_pDevice->GetDeviceHandle(), &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));
	*/
}

void VulkanGlobal::InitPipeline()
{
}

void VulkanGlobal::InitShaderModule()
{
}

void VulkanGlobal::InitDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> descPoolSize =
	{
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10
		},
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10
		}
	};

	VkDescriptorPoolCreateInfo descPoolInfo = {};
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pPoolSizes = descPoolSize.data();
	descPoolInfo.poolSizeCount = descPoolSize.size();
	descPoolInfo.maxSets = 10;

	m_pDescriptorPool = DescriptorPool::Create(m_pDevice, descPoolInfo);
}

void VulkanGlobal::InitDescriptorSet()
{
}

void VulkanGlobal::InitDrawCmdBuffers()
{
	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
		m_perFrameRes.push_back(FrameMgr()->AllocatePerFrameResource(i));
}

void VulkanGlobal::InitSemaphore()
{
}

void VulkanGlobal::InitMaterials()
{
	// Gun material
	std::vector<UniformVar> vars =
	{ 
		{
			{
				Vec4Unit,
				"Albedo Roughness"
			},
			{
				Vec2Unit,
				"AO Metalic"
			},
			{
				OneUnit,
				"Albedo Roughness Texture Index"
			},
			{
				OneUnit,
				"Normal AO Texture Index"
			},
			{
				OneUnit,
				"Metallic Texture Index"
			}
		}
	};

	/*SimpleMaterialCreateInfo info = {};
	info.shaderPaths			= { L"../data/shaders/pbr.vert.spv", L"", L"", L"", L"../data/shaders/pbr.frag.spv", L"" };
	info.vertexBindingsInfo		= { m_pGunMesh->GetVertexBuffer()->GetBindingDesc() };
	info.vertexAttributesInfo	= m_pGunMesh->GetVertexBuffer()->GetAttribDesc();
	info.materialUniformVars	= vars;
	info.pRenderPass			= RenderPassDiction::GetInstance()->GetForwardRenderPass()->GetRenderPass();
	info.vertexFormat			= m_pGunMesh->GetVertexBuffer()->GetVertexFormat();
	info.isDeferredShadingMaterial = false;


	m_pGunMaterial = ForwardMaterial::CreateDefaultMaterial(info);
	m_pGunMaterialInstance = m_pGunMaterial->CreateMaterialInstance();
	m_pGunMaterialInstance->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pGunMaterialInstance->SetParameter(0, Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
	m_pGunMaterialInstance->SetParameter(1, Vector2f(1.0f, 1.0f));
	m_pGunMaterialInstance->SetMaterialTexture(2, RGBA8_1024, "GunAlbedoRoughness");
	m_pGunMaterialInstance->SetMaterialTexture(3, RGBA8_1024, "GunNormalAO");
	m_pGunMaterialInstance->SetMaterialTexture(4, R8_1024, "GunMetallic");*/

	SimpleMaterialCreateInfo info = {};
	info.shaderPaths = { L"../data/shaders/pbr_gbuffer_gen.vert.spv", L"", L"", L"", L"../data/shaders/pbr_gbuffer_gen.frag.spv", L"" };
	info.vertexBindingsInfo = { m_pGunMesh->GetVertexBuffer()->GetBindingDesc() };
	info.vertexAttributesInfo = m_pGunMesh->GetVertexBuffer()->GetAttribDesc();
	info.materialUniformVars = vars;
	info.pRenderPass = RenderPassDiction::GetInstance()->GetDeferredRenderPass()->GetRenderPass();
	info.vertexFormat = m_pGunMesh->GetVertexBuffer()->GetVertexFormat();
	info.isDeferredShadingMaterial = false;


	m_pGunMaterial = DeferredMaterial::CreateDefaultMaterial(info);
	m_pGunMaterialInstance = m_pGunMaterial->CreateMaterialInstance();
	m_pGunMaterialInstance->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pGunMaterialInstance->SetParameter(0, Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
	m_pGunMaterialInstance->SetParameter(1, Vector2f(1.0f, 1.0f));
	m_pGunMaterialInstance->SetMaterialTexture(2, RGBA8_1024, "GunAlbedoRoughness");
	m_pGunMaterialInstance->SetMaterialTexture(3, RGBA8_1024, "GunNormalAO");
	m_pGunMaterialInstance->SetMaterialTexture(4, R8_1024, "GunMetallic");

	m_pSphereMaterialInstance = m_pGunMaterial->CreateMaterialInstance();
	m_pSphereMaterialInstance->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pSphereMaterialInstance->SetParameter(0, Vector4f(1.0f, 0.0f, 0.0f, 0.1f));
	m_pSphereMaterialInstance->SetParameter(1, Vector2f(1.0f, 0.1f));
	m_pSphereMaterialInstance->SetMaterialTexture(2, RGBA8_1024, ":)");
	m_pSphereMaterialInstance->SetMaterialTexture(3, RGBA8_1024, ":)");
	m_pSphereMaterialInstance->SetMaterialTexture(4, R8_1024, ":)");

	// Skybox material
	vars = {};

	info.shaderPaths			= { L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/sky_box.frag.spv", L"" };
	info.vertexBindingsInfo		= { m_pCubeMesh->GetVertexBuffer()->GetBindingDesc() };
	info.vertexAttributesInfo	= m_pCubeMesh->GetVertexBuffer()->GetAttribDesc();
	info.materialUniformVars	= vars;
	info.pRenderPass			= RenderPassDiction::GetInstance()->GetForwardRenderPass()->GetRenderPass();
	info.vertexFormat			= m_pCubeMesh->GetVertexBuffer()->GetVertexFormat();

	m_pSkyBoxMaterial = ForwardMaterial::CreateDefaultMaterial(info);
	m_pSkyBoxMaterialInstance = m_pSkyBoxMaterial->CreateMaterialInstance();
	m_pSkyBoxMaterialInstance->SetRenderMask(1 << RenderWorkManager::Scene);

	/*layout =
	{
		{
			DynamicUniformBuffer,
			"MaterialVariables",
			{
				{ Vec4Unit, "TestColor" },
			}
		},
		{ CombinedSampler, "tex0" },
		{ CombinedSampler, "tex1" },
	};

	info.shaderPaths = { L"../data/shaders/screen_quad.vert.spv", L"", L"", L"", L"../data/shaders/screen_quad.frag.spv", L"" };
	info.vertexBindingsInfo = { m_pQuadMesh->GetVertexBuffer()->GetBindingDesc() };
	info.vertexAttributesInfo = m_pQuadMesh->GetVertexBuffer()->GetAttribDesc();
	info.maxMaterialInstance = 1;
	info.materialVariableLayout = layout;
	info.pRenderPass = RenderWorkManager::GetDefaultRenderPass();

	m_pTestMaterial = Material::CreateDefaultMaterial(info);
	m_pTestMaterialInstance = m_pTestMaterial->CreateMaterialInstance();
	m_pTestMaterialInstance->SetRenderMask(1 << GlobalVulkanStates::Scene);
	m_pTestMaterialInstance->SetMaterialTexture(1, m_pPrefilterEnvTex);
	m_pTestMaterialInstance->SetMaterialTexture(2, m_pBRDFLut);*/
}

void VulkanGlobal::InitScene()
{
	CameraInfo camInfo =
	{
		3.1415f / 3.0f,
		1024.0f / 768.0f,
		1.0f,
		2000.0f,
	};
	m_pCameraComp = Camera::Create(camInfo);
	m_pCameraObj = BaseObject::Create();
	m_pCameraObj->AddComponent(m_pCameraComp);

	camInfo =
	{
		3.1415f / 2.0f,
		OffScreenSize / OffScreenSize,
		1.0f,
		2000.0f,
	};

	m_pCharacter = Character::Create({ 100.0f }, m_pCameraComp);
	m_pCameraObj->AddComponent(m_pCharacter);

	m_pCameraObj->SetPos({ 0, 0, 50 });
	m_pCameraObj->Update();

	m_pGunObject = BaseObject::Create();
	m_pGunObject1 = BaseObject::Create();
	m_pSphere = BaseObject::Create();
	m_pGunMeshRenderer = MeshRenderer::Create(m_pGunMesh, m_pGunMaterialInstance);
	m_pGunMeshRenderer->SetDescription(L"GunMeshRenderer0");
	m_pGunMeshRenderer1 = MeshRenderer::Create(m_pGunMesh, m_pGunMaterialInstance);
	m_pGunMeshRenderer1->SetDescription(L"GunMeshRenderer1");
	m_pSphereRenderer = MeshRenderer::Create(m_pSphereMesh, m_pSphereMaterialInstance);
	m_pSphereRenderer->SetDescription(L"SphereRenderer");
	m_pGunObject->AddComponent(m_pGunMeshRenderer);
	m_pGunObject1->AddComponent(m_pGunMeshRenderer1);
	m_pGunObject1->SetPos({-100, 0, 0});
	m_pSphere->AddComponent(m_pSphereRenderer);
	m_pSphere->SetPos(0, 100, 0);

	m_pSkyBoxObject = BaseObject::Create();
	m_pSkyBoxMeshRenderer = MeshRenderer::Create(m_pCubeMesh, { m_pSkyBoxMaterialInstance });
	m_pSkyBoxObject->AddComponent(m_pSkyBoxMeshRenderer);

	/*
	m_pTestObject = BaseObject::Create();
	m_pTestRenderer = MeshRenderer::Create(m_pQuadMesh, m_pTestMaterialInstance);
	m_pTestObject->AddComponent(m_pTestRenderer);*/

	m_pRootObject = BaseObject::Create();
	m_pRootObject->AddChild(m_pGunObject);
	m_pRootObject->AddChild(m_pGunObject1);
	m_pGunObject->AddChild(m_pSphere);
	//m_pRootObject->AddChild(m_pTestObject);
	m_pRootObject->AddChild(m_pSkyBoxObject);

	m_pRootObject->AddChild(m_pCameraObj);

	UniformData::GetInstance()->GetGlobalUniforms()->SetMainLightColor({ 1, 1, 1 });
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainLightDir({ -1, -1, 1 });
	UniformData::GetInstance()->GetGlobalUniforms()->SetRenderSettings({ 1.0f / 2.2f, 4.5f, 11.2f, 0.0f });
}

void VulkanGlobal::EndSetup()
{
	GlobalDeviceObjects::GetInstance()->GetStagingBufferMgr()->FlushDataMainThread();
}

void VulkanGlobal::Draw()
{
	GetSwapChain()->AcquireNextImage();

	m_pRootObject->Update();
	m_pRootObject->LateUpdate();
	UniformData::GetInstance()->SyncDataBuffer();
	m_pGunMaterial->SyncBufferData();
	
	RenderWorkManager::GetInstance()->SetRenderState(RenderWorkManager::Scene);
	GetGlobalVulkanStates()->RestoreViewport();
	GetGlobalVulkanStates()->RestoreScissor();

	m_pCharacter->Move(m_moveFlag, 0.001f);

	RenderWorkManager::GetInstance()->SetCurrentFrameBuffer(RenderWorkManager::Deferred);

	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = m_perFrameRes[FrameMgr()->FrameIndex()]->AllocatePrimaryCommandBuffer();
	pDrawCmdBuffer->StartPrimaryRecording();
	
	std::vector<VkClearValue> clearValues =
	{
		{ 0.2f, 0.2f, 0.2f, 0.2f },
		{ 1.0f, 0 }
	};

	RenderPassDiction::GetInstance()->GetDeferredRenderPass()->BeginRenderPass(pDrawCmdBuffer, RenderWorkManager::GetInstance()->GetCurrentFrameBuffer());

	RenderPassDiction::GetInstance()->GetDeferredRenderPass()->BeginGeometryPass(pDrawCmdBuffer);

	m_pGunMaterial->OnPassStart();
	m_pGunMaterial->Draw(pDrawCmdBuffer);
	m_pGunMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetDeferredRenderPass()->EndGeometryPass(pDrawCmdBuffer);

	RenderPassDiction::GetInstance()->GetDeferredRenderPass()->BeginShadingPass(pDrawCmdBuffer);
	RenderPassDiction::GetInstance()->GetDeferredRenderPass()->EndShadingPass(pDrawCmdBuffer);

	RenderPassDiction::GetInstance()->GetDeferredRenderPass()->BeginTransparentPass(pDrawCmdBuffer);
	RenderPassDiction::GetInstance()->GetDeferredRenderPass()->EndTransparentPass(pDrawCmdBuffer);

	RenderPassDiction::GetInstance()->GetDeferredRenderPass()->EndRenderPass(pDrawCmdBuffer, RenderWorkManager::GetInstance()->GetCurrentFrameBuffer());

	//m_pSkyBoxMaterial->OnPassStart();
	//m_pSkyBoxMaterial->Draw(pDrawCmdBuffer);
	m_pSkyBoxMaterial->OnPassEnd();

	pDrawCmdBuffer->EndPrimaryRecording();
	FrameMgr()->CacheSubmissioninfo(GlobalGraphicQueue(), { pDrawCmdBuffer }, {}, false);
	
	GetSwapChain()->QueuePresentImage(GlobalObjects()->GetPresentQueue());
}

void VulkanGlobal::InitVulkan(HINSTANCE hInstance, WNDPROC wndproc)
{
	SetupWindow(hInstance, wndproc);
	InitVulkanInstance();
	InitPhysicalDevice(m_hPlatformInst, m_hWindow);
	InitSurface();
	InitVulkanDevice();
	GlobalDeviceObjects::GetInstance()->InitObjects(m_pDevice);
	InitSwapchain();
	InitQueue();

	InitCommandPool();
	InitSetupCommandBuffer();
	InitMemoryMgr();
	InitSwapchainImgs();
	InitDepthStencil();
	InitRenderpass();
	InitFrameBuffer();
	InitVertices();
	InitUniforms();
	InitDescriptorSetLayout();
	InitShaderModule();
	InitPipelineCache();
	InitPipeline();
	InitDescriptorPool();
	InitDescriptorSet();
	InitDrawCmdBuffers();
	InitSemaphore();
	InitMaterials();
	InitScene();
	EndSetup();
}