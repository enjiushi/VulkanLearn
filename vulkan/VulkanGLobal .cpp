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
#include "../common/Util.h"
#include "../common/Enums.h"
#include "../class/GlobalTextures.h"
#include "../scene/SceneGenerator.h"
#include "../class/RenderPassDiction.h"
#include "../class/ForwardRenderPass.h"
#include "../class/GBufferPass.h"
#include "../class/DeferredShadingPass.h"
#include "../class/InputHub.h"
#include "../class/Timer.h"
#include "../component/FrustumJitter.h"

bool PREBAKE_CB = true;

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

		if ((FrameBufferDiction::WINDOW_WIDTH != screenWidth) && (FrameBufferDiction::WINDOW_HEIGHT != screenHeight))
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
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = fullscreen ? (long)screenWidth : (long)FrameBufferDiction::WINDOW_WIDTH;
	windowRect.bottom = fullscreen ? (long)screenHeight : (long)FrameBufferDiction::WINDOW_HEIGHT;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	std::string windowTitle = PROJECT_NAME;
	m_hWindow = CreateWindowEx(0,
		PROJECT_NAME,
		windowTitle.c_str(),
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0,
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hinstance,
		NULL);

	if (!fullscreen)
	{
		// Center on screen
		uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(m_hWindow, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

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
#define KEY_Q 0x51
#define KEY_E 0x45
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
		default:
			InputHub::GetInstance()->ProcessKey(KEY_DOWN, wParam);
			break;
		}
		break;
	case WM_KEYUP:
		InputHub::GetInstance()->ProcessKey(KEY_UP, wParam);
		break;
	case WM_RBUTTONDOWN:
		InputHub::GetInstance()->ProcessMouse(KEY_DOWN, { (float)LOWORD(lParam), (float)HIWORD(lParam) });
		break;
	case WM_RBUTTONUP:
		InputHub::GetInstance()->ProcessMouse(KEY_UP, { (float)LOWORD(lParam), (float)HIWORD(lParam) });
		break;
	case WM_MOUSEMOVE:
		InputHub::GetInstance()->ProcessMouse({ (float)LOWORD(lParam), (float)HIWORD(lParam) });
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
	static std::chrono::time_point<std::chrono::steady_clock> startTime, endTime, initTime;
	startTime = std::chrono::high_resolution_clock::now();
	initTime = startTime;
	MSG msg;
	bool quitMessageReceived = false;
	while (!quitMessageReceived)
	{
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
		auto endTime = std::chrono::high_resolution_clock::now();
		Timer::ElapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
		Timer::TotalTime = std::chrono::duration<double, std::milli>(endTime - initTime).count();
		startTime = endTime;
		Draw();
		frameCount++;

		fpsTimer += (float)Timer::ElapsedTime;

		if (fpsTimer > 1000.0f)
		{
			std::stringstream ss;
			ss << "Elapsed Time:" << 1000.0f / frameCount;
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
}

void VulkanGlobal::InitRenderpass()
{
}

void VulkanGlobal::InitFrameBuffer()
{
}

void VulkanGlobal::InitVertices()
{
	m_pGunMesh = Mesh::Create("../data/textures/cerberus/cerberus.fbx", VertexFormatPNTCT);
	m_pSphereMesh = Mesh::Create("../data/models/sphere.obj", VertexFormatPNTCT);
	m_pQuadMesh = SceneGenerator::GeneratePBRQuadMesh();

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

	m_pPBRBoxMesh = SceneGenerator::GeneratePBRBoxMesh();
}

// Replace rgbTex's alpha channel with rTex's red channel
// This utility function is used only here to achieve texture packing as a preparation for later texture array implementation
void CombineRGBA8_R8_RGBA8(gli::texture2d& rgbaTex, gli::texture2d rTex)
{

	ASSERTION(rgbaTex.extent().x == rTex.extent().x && rgbaTex.extent().y == rTex.extent().y);
	ASSERTION(rgbaTex.layers() == rTex.layers() && rgbaTex.levels() == rTex.levels());

	uint8_t* rgbaTexData = (uint8_t*)rgbaTex.data();
	uint8_t* rTexData = (uint8_t*)rTex.data();

	for (uint32_t i = 0; i < rTex.size(); i++)
	{
		rgbaTexData[i * 4 + 3] = rTexData[i];
	}
}

void SetAlphaChannel(gli::texture2d& rgbaTex, uint8_t alpha)
{
	uint8_t* rgbaTexData = (uint8_t*)rgbaTex.data();

	for (uint32_t i = 0; i < rgbaTex.size() / 4; i++)
	{
		rgbaTexData[i * 4 + 3] = alpha;
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

	gli::texture2d texCheckerTex(gli::load("../data/textures/tex_checker.ktx"));
	SetAlphaChannel(texCheckerTex, 0.9f * 255);

	gli::texture2d blueNoise(gli::load("../data/textures/blue_noise_1024.ktx"));

	gli::texture2d gliMetalic(gli::load("../data/textures/cerberus/metallic_1024.ktx"));

	m_pAlbedoRoughness = Texture2D::Create(m_pDevice, { {gliAlbedoTex} }, VK_FORMAT_R8G8B8A8_UNORM);
	m_pMetalic = Texture2D::Create(m_pDevice, { {gliMetalic} }, VK_FORMAT_R8_UNORM);
	m_pNormalAO = Texture2D::Create(m_pDevice, { {gliNormalTex} }, VK_FORMAT_R8G8B8A8_UNORM);

	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "GunAlbedoRoughness", "", "RGB:Albedo, A:Roughness" }, gliAlbedoTex);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "GunNormalAO", "", "RGB:Normal, A:AO" }, gliNormalTex);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "TexChecker", "", "Texture checker board" }, texCheckerTex);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "BlueNoise", "", "Blue Noise" }, blueNoise);
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
		m_perFrameRes.push_back(FrameMgr()->AllocatePerFrameResource(i, !PREBAKE_CB));
}

void VulkanGlobal::InitSemaphore()
{
}

void VulkanGlobal::InitMaterials()
{
	m_pGunMaterialInstance = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pGunMaterialInstance->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pGunMaterialInstance->SetParameter("AlbedoRoughness", Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
	m_pGunMaterialInstance->SetParameter("AOMetalic", Vector2f(1.0f, 1.0f));
	m_pGunMaterialInstance->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, "GunAlbedoRoughness");
	m_pGunMaterialInstance->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, "GunNormalAO");
	m_pGunMaterialInstance->SetMaterialTexture("MetallicTextureIndex", R8_1024, "GunMetallic");

	m_pSphereMaterialInstance0 = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pSphereMaterialInstance0->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pSphereMaterialInstance0->SetParameter("AlbedoRoughness", Vector4f(1.0f, 0.0f, 0.0f, 0.1f));
	m_pSphereMaterialInstance0->SetParameter("AOMetalic", Vector2f(1.0f, 0.1f));
	m_pSphereMaterialInstance0->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
	m_pSphereMaterialInstance0->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	m_pSphereMaterialInstance0->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_pSphereMaterialInstance1 = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pSphereMaterialInstance1->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pSphereMaterialInstance1->SetParameter("AlbedoRoughness", Vector4f(1.0f, 1.0f, 1.0f, 0.1f));
	m_pSphereMaterialInstance1->SetParameter("AOMetalic", Vector2f(1.0f, 1.0f));
	m_pSphereMaterialInstance1->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
	m_pSphereMaterialInstance1->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	m_pSphereMaterialInstance1->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_pSphereMaterialInstance2 = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pSphereMaterialInstance2->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pSphereMaterialInstance2->SetParameter("AlbedoRoughness", Vector4f(0.0f, 1.0f, 0.0f, 1.0f));
	m_pSphereMaterialInstance2->SetParameter("AOMetalic", Vector2f(1.0f, 0.1f));
	m_pSphereMaterialInstance2->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
	m_pSphereMaterialInstance2->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	m_pSphereMaterialInstance2->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_pQuadMaterialInstance = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pQuadMaterialInstance->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pQuadMaterialInstance->SetParameter("AlbedoRoughness", Vector4f(0.7f, 0.7f, 0.7f, 0.92f));
	m_pQuadMaterialInstance->SetParameter("AOMetalic", Vector2f(1.0f, 0.99f));
	m_pQuadMaterialInstance->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
	m_pQuadMaterialInstance->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	m_pQuadMaterialInstance->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_pBoxMaterialInstance0 = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pBoxMaterialInstance0->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pBoxMaterialInstance0->SetParameter("AlbedoRoughness", Vector4f(0.0f, 1.0f, 0.0f, 0.9f));
	m_pBoxMaterialInstance0->SetParameter("AOMetalic", Vector2f(1.0f, 0.1f));
	m_pBoxMaterialInstance0->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, "TexChecker");
	m_pBoxMaterialInstance0->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	m_pBoxMaterialInstance0->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_pBoxMaterialInstance1 = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pBoxMaterialInstance1->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pBoxMaterialInstance1->SetParameter("AlbedoRoughness", Vector4f(0.0f, 0.0f, 1.0f, 0.1f));
	m_pBoxMaterialInstance1->SetParameter("AOMetalic", Vector2f(1.0f, 0.9f));
	m_pBoxMaterialInstance1->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
	m_pBoxMaterialInstance1->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	m_pBoxMaterialInstance1->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_pBoxMaterialInstance2 = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pBoxMaterialInstance2->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pBoxMaterialInstance2->SetParameter("AlbedoRoughness", Vector4f(1.0f, 1.0f, 0.0f, 0.5f));
	m_pBoxMaterialInstance2->SetParameter("AOMetalic", Vector2f(1.0f, 0.9f));
	m_pBoxMaterialInstance2->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
	m_pBoxMaterialInstance2->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	m_pBoxMaterialInstance2->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_pSkyBoxMaterialInstance = RenderWorkManager::GetInstance()->AcquireSkyBoxMaterialInstance();

	m_pShadowMapMaterialInstance = RenderWorkManager::GetInstance()->AcquireShadowMaterialInstance();
}

void VulkanGlobal::InitScene()
{
	CameraInfo camInfo =
	{
		3.1415f / 3.0f,
		1536.0f / 1024.0f,
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

	m_pCharacter = Character::Create({ 0.3f, 0.002f }, m_pCameraComp);
	m_pCameraObj->AddComponent(m_pCharacter);
	m_pCameraObj->AddComponent(FrustumJitter::Create());

	m_pCameraObj->SetPos({ 0, 100, -220 });
	m_pCameraObj->SetRotation(Matrix3f::EulerAngle(-0.78f, 3.14f, 0));

	m_pDirLightObj = BaseObject::Create();
	m_pDirLightObj->SetPos(64, 64, -64);
	m_pDirLightObj->SetRotation(Matrix3f::EulerAngle(0.78f, 0, 0) * Matrix3f::EulerAngle(0, 2.355f, 0));

	m_pDirLight = DirectionLight::Create({ 4.0f, 4.0f, 4.0f });
	m_pDirLightObj->AddComponent(m_pDirLight);

	m_pGunObject = BaseObject::Create();
	m_pSphere0 = BaseObject::Create();
	m_pSphere1 = BaseObject::Create();
	m_pSphere2 = BaseObject::Create();
	m_pQuadObject = BaseObject::Create();
	m_pBoxObject0 = BaseObject::Create();
	m_pBoxObject1 = BaseObject::Create();
	m_pBoxObject2 = BaseObject::Create();

	m_pGunMeshRenderer = MeshRenderer::Create(m_pGunMesh, { m_pGunMaterialInstance, m_pShadowMapMaterialInstance });
	m_pSphereRenderer0 = MeshRenderer::Create(m_pSphereMesh, { m_pSphereMaterialInstance0, m_pShadowMapMaterialInstance });
	m_pSphereRenderer1 = MeshRenderer::Create(m_pSphereMesh, { m_pSphereMaterialInstance1, m_pShadowMapMaterialInstance });
	m_pSphereRenderer2 = MeshRenderer::Create(m_pSphereMesh, { m_pSphereMaterialInstance2, m_pShadowMapMaterialInstance });
	m_pQuadRenderer = MeshRenderer::Create(m_pQuadMesh, { m_pQuadMaterialInstance, m_pShadowMapMaterialInstance });
	m_pBoxRenderer0 = MeshRenderer::Create(m_pPBRBoxMesh, { m_pBoxMaterialInstance0, m_pShadowMapMaterialInstance });
	m_pBoxRenderer1 = MeshRenderer::Create(m_pPBRBoxMesh, { m_pBoxMaterialInstance1, m_pShadowMapMaterialInstance });
	m_pBoxRenderer2 = MeshRenderer::Create(m_pPBRBoxMesh, { m_pBoxMaterialInstance2, m_pShadowMapMaterialInstance });

	m_pGunObject->AddComponent(m_pGunMeshRenderer);
	m_pGunObject->SetPos({ -80, -8, 0 });

	m_pSphere0->AddComponent(m_pSphereRenderer0);
	m_pSphere0->SetPos(40, -15, 0);

	m_pSphere1->AddComponent(m_pSphereRenderer1);
	m_pSphere1->SetPos(100, -15, 0);

	m_pSphere2->AddComponent(m_pSphereRenderer2);
	m_pSphere2->SetPos(100, -15, 60);

	m_pQuadObject->AddComponent(m_pQuadRenderer);
	m_pQuadObject->SetPos(-50, -40, 0);
	m_pQuadObject->SetScale(200);

	m_pBoxObject0->AddComponent(m_pBoxRenderer0);
	m_pBoxObject0->SetScale(30);
	m_pBoxObject0->SetPos(-20, -10, 0);

	m_pBoxObject1->AddComponent(m_pBoxRenderer1);
	m_pBoxObject1->SetScale(15);
	m_pBoxObject1->SetPos(-20, 35, 0);

	m_pBoxObject2->AddComponent(m_pBoxRenderer2);
	m_pBoxObject2->SetScale(15);
	m_pBoxObject2->SetPos(-20, -25, 50);

	Quaternionf rot = Quaternionf(Vector3f(1, 0, 0), 0);
	m_pQuadObject->SetRotation(Quaternionf(Vector3f(1, 0, 0), -1.57));

	m_pSkyBoxObject = BaseObject::Create();
	m_pSkyBoxMeshRenderer = MeshRenderer::Create(m_pCubeMesh, { m_pSkyBoxMaterialInstance });
	m_pSkyBoxObject->AddComponent(m_pSkyBoxMeshRenderer);

	m_pRootObject = BaseObject::Create();
	m_pRootObject->AddChild(m_pGunObject);
	m_pRootObject->AddChild(m_pSphere0);
	m_pRootObject->AddChild(m_pSphere1);
	m_pRootObject->AddChild(m_pSphere2);
	m_pRootObject->AddChild(m_pQuadObject);
	m_pRootObject->AddChild(m_pBoxObject0);
	m_pRootObject->AddChild(m_pBoxObject1);
	m_pRootObject->AddChild(m_pBoxObject2);
	m_pRootObject->AddChild(m_pSkyBoxObject);
	m_pRootObject->AddChild(m_pDirLightObj);

	m_pRootObject->AddChild(m_pCameraObj);

	UniformData::GetInstance()->GetGlobalUniforms()->SetMainLightColor({ 1, 1, 1 });
	UniformData::GetInstance()->GetGlobalUniforms()->SetMainLightDir({ 1, 1, -1 });
	UniformData::GetInstance()->GetGlobalUniforms()->SetRenderSettings({ 1.0f / 2.2f, 4.5f, 11.2f, 0.0f });
}

class RoughnessChanger : public IInputListener
{
public:
	void ProcessKey(KeyState keyState, uint8_t keyCode) override;
	void ProcessMouse(KeyState keyState, const Vector2f& mousePosition) override {}
	void ProcessMouse(const Vector2f& mousePosition) override {}
	float roughness = 0.5f;
};

void RoughnessChanger::ProcessKey(KeyState keyState, uint8_t keyCode)
{
	static float interval = 0.01f;
	if (keyCode == KEY_F)
	{
		roughness += interval;
		roughness = roughness > 1.0f ? 1.0f : roughness;
	}
	if (keyCode == KEY_L)
	{
		roughness -= interval;
		roughness = roughness < 0.05f ? 0.05f : roughness;
	}
}

std::shared_ptr<RoughnessChanger> c;

void VulkanGlobal::EndSetup()
{
	GlobalDeviceObjects::GetInstance()->GetStagingBufferMgr()->FlushDataMainThread();
	m_commandBufferList.resize(GetSwapChain()->GetSwapChainImageCount() * 2);

	m_pRootObject->Awake();
	m_pRootObject->Start();

	c = std::make_shared<RoughnessChanger>();
	InputHub::GetInstance()->Register(c);
}

void VulkanGlobal::Draw()
{
	static uint32_t pingpong = 0;


	GetSwapChain()->AcquireNextImage();

	uint32_t frameIndex = FrameMgr()->FrameIndex();
	uint32_t cbIndex = frameIndex * 2 + pingpong;
	UniformData::GetInstance()->GetPerFrameUniforms()->SetDeltaTime(Timer::ElapsedTime);
	UniformData::GetInstance()->GetPerFrameUniforms()->SetSinTime(std::sinf(Timer::TotalTime));
	UniformData::GetInstance()->GetPerFrameUniforms()->SetFrameIndex(frameIndex);

	RenderWorkManager::GetInstance()->SetRenderStateMask((1 << RenderWorkManager::Scene) | (1 << RenderWorkManager::ShadowMapGen));

	m_pQuadMaterialInstance->SetParameter("AlbedoRoughness", Vector4f(0.7f, 0.7f, 0.7f, c->roughness));

	m_pRootObject->Update();
	m_pRootObject->LateUpdate();

	UniformData::GetInstance()->SyncDataBuffer();
	RenderWorkManager::GetInstance()->SyncMaterialData();

	RenderWorkManager::GetInstance()->OnFrameBegin();

	if (!PREBAKE_CB || m_commandBufferList[cbIndex] == nullptr)
	{
		std::shared_ptr<CommandBuffer> pDrawCmdBuffer = m_perFrameRes[FrameMgr()->FrameIndex()]->AllocatePrimaryCommandBuffer();
		pDrawCmdBuffer->StartPrimaryRecording();

		RenderWorkManager::GetInstance()->Draw(pDrawCmdBuffer, pingpong);

		pDrawCmdBuffer->EndPrimaryRecording();

		m_commandBufferList[cbIndex] = pDrawCmdBuffer;
	}

	RenderWorkManager::GetInstance()->OnFrameEnd();

	FrameMgr()->CacheSubmissioninfo(GlobalGraphicQueue(), { m_commandBufferList[cbIndex] }, {}, false);
	
	GetSwapChain()->QueuePresentImage(GlobalObjects()->GetPresentQueue());

	pingpong = (pingpong + 1) % 2;
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