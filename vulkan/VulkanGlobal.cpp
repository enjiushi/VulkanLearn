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
#include "../class/AssimpSceneReader.h"
#include "../component/AnimationController.h"
#include "../class/PerFrameData.h"
#include "../class/FrameEventManager.h"

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
#define KEY_G 0x47
#define KEY_K 0x4B
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
			InputHub::GetInstance()->ProcessKey(KEY_DOWN, (uint8_t)wParam);
			break;
		}
		break;
	case WM_KEYUP:
		InputHub::GetInstance()->ProcessKey(KEY_UP, (uint8_t)wParam);
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
	static double fpsTimer = 0;
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
		Timer::SetElapsedTime(std::chrono::duration<double, std::milli>(endTime - startTime).count());
		startTime = endTime;
		Draw();
		frameCount++;

		fpsTimer += (float)Timer::GetElapsedTime();

		if (fpsTimer > 1000.0)
		{
			std::stringstream ss;
			ss << "Elapsed Time:" << 1000.0 / frameCount;
			SetWindowText(m_hWindow, ss.str().c_str());
			fpsTimer = 0.0;
			frameCount = 0;
		}
	}
#endif
}

void VulkanGlobal::InitCommandPool()
{
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
	m_LODPatchLevel = 3;

	m_pLODTriangleMesh = SceneGenerator::GenerateLODTriangleMesh(m_LODPatchLevel, true);

	m_pQuadMesh = SceneGenerator::GeneratePBRQuadMesh();

	m_pCubeMesh = SceneGenerator::GenerateBoxMesh();

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

void CombineRGBA8_RGBA8(gli::texture2d& rgba_rgbTex, gli::texture2d rgba_aTex, bool revert = false, uint32_t whichChannel = 3)
{

	ASSERTION(rgba_rgbTex.extent().x == rgba_aTex.extent().x && rgba_rgbTex.extent().y == rgba_aTex.extent().y);
	ASSERTION(rgba_rgbTex.layers() == rgba_aTex.layers() && rgba_rgbTex.levels() == rgba_aTex.levels());

	uint8_t* rgba_rgbTexData = (uint8_t*)rgba_rgbTex.data();
	uint8_t* rgba_aTexData = (uint8_t*)rgba_aTex.data();

	for (uint32_t i = 0; i < rgba_aTex.size() / 4; i++)
	{
		if (revert)
			rgba_rgbTexData[i * 4 + 3] = 255 - rgba_aTexData[i * 4 + whichChannel];
		else
			rgba_rgbTexData[i * 4 + 3] = rgba_aTexData[i * 4 + whichChannel];
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

gli::texture2d ExtractAlphaChannel(gli::texture2d rgbaTex)
{
	gli::texture2d alphaTex(gli::FORMAT_R8_UNORM_PACK8, rgbaTex.extent(), rgbaTex.levels());

	uint8_t* rTexData = (uint8_t*)alphaTex.data();
	uint8_t* rgbaTexData = (uint8_t*)rgbaTex.data();

	for (uint32_t i = 0; i < alphaTex.size(); i++)
	{
		rTexData[i] = rgbaTexData[i * 4 + 3];
	}
	return alphaTex;
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
	SetAlphaChannel(texCheckerTex, (uint8_t)(0.9f * 255));

	gli::texture2d blueNoise(gli::load("../data/textures/blue_noise_1024.ktx"));

	gli::texture2d gliMetalic(gli::load("../data/textures/cerberus/metallic_1024.ktx"));

	gli::texture2d gliAluminumAlbedo(gli::load("../data/textures/aluminum_albedo_1024.ktx"));

	gli::texture2d gliTempTex(gli::load("../data/textures/aluminum_metalness_1024.ktx"));
	gli::texture2d gliAluminumMetalic = ExtractAlphaChannel(gliTempTex);
	gli::texture2d gliAluminumNormalAO(gli::load("../data/textures/aluminum_normal_1024.ktx"));
	SetAlphaChannel(gliAluminumNormalAO, (uint8_t)(1.0f * 255));
	CombineRGBA8_R8_RGBA8(gliAluminumAlbedo, gliAluminumMetalic);

	gli::texture2d gliCamDirt(gli::load("../data/textures/cam_dirt_1024.ktx"));

	gli::texture2d gliSophiaAlbedoRoughness(gli::load("../data/textures/sophia_albedo_1024.ktx"));
	gli::texture2d gliSophiaRoughness(gli::load("../data/textures/sophia_gloss_1024.ktx"));
	CombineRGBA8_RGBA8(gliSophiaAlbedoRoughness, gliSophiaRoughness, true, 0);
	//SetAlphaChannel(gliSophiaAlbedoRoughness, 0.0f * 255);

	gli::texture2d gliSophiaNormal(gli::load("../data/textures/sophia_normal_1024.ktx"));
	SetAlphaChannel(gliSophiaNormal, (uint8_t)(1.0f * 255));

	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "GunAlbedoRoughness", "", "RGB:Albedo, A:Roughness" }, gliAlbedoTex);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "GunNormalAO", "", "RGB:Normal, A:AO" }, gliNormalTex);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "TexChecker", "", "Texture checker board" }, texCheckerTex);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "BlueNoise", "", "Blue Noise" }, blueNoise);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "AluminumNormalAO", "", "Aluminum plate normal ao map" }, gliAluminumNormalAO);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::R8_1024, { "GunMetallic", "", "R:Metalic" }, gliMetalic);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::R8_1024, { "AluminumMetalic", "", "Aluminum plate metalic map" }, gliAluminumMetalic);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "AluminumAlbedoRoughness", "", "Aluminum plate albedo roughness" }, gliAluminumAlbedo);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "CamDirt0", "", "Camera dirt texture 0" }, gliCamDirt);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "SophiaAlbedoRoughness", "", "Sophia model albedo" }, gliSophiaAlbedoRoughness);
	UniformData::GetInstance()->GetGlobalTextures()->InsertTexture(InGameTextureType::RGBA8_1024, { "SophiaNormalAO", "", "Sophia model normal" }, gliSophiaNormal);

	gli::texture_cube gliSkyBox(gli::load("../data/textures/hdr/gcanyon_cube.ktx"));
	UniformData::GetInstance()->GetGlobalTextures()->InitIBLTextures(gliSkyBox);

	UniformData::GetInstance()->GetGlobalTextures()->InsertScreenSizeTexture({ "MipmapTemporalResult", "", "Mip map temporal result, used for next frame ssr" });
}

void VulkanGlobal::InitDescriptorSetLayout()
{
}

void VulkanGlobal::InitPipelineCache()
{
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
	descPoolInfo.poolSizeCount = (uint32_t)descPoolSize.size();
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

	m_pSphereMaterialInstance3 = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pSphereMaterialInstance3->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pSphereMaterialInstance3->SetParameter("AlbedoRoughness", Vector4f(0.0f, 1.0f, 0.0f, 1.0f));
	m_pSphereMaterialInstance3->SetParameter("AOMetalic", Vector2f(1.0f, 0.1f));
	m_pSphereMaterialInstance3->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
	m_pSphereMaterialInstance3->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	m_pSphereMaterialInstance3->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	for (uint32_t i = 0; i < 2; i++)
	{
		std::shared_ptr<MaterialInstance> pInst = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();

		if (i == 0)
		{
			pInst->SetRenderMask(1 << RenderWorkManager::Scene);
			pInst->SetParameter("AlbedoRoughness", Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
			pInst->SetParameter("AOMetalic", Vector2f(1.0f, 0.1f));
			pInst->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
			pInst->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
			pInst->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");
		}
		else
		{
			pInst->SetRenderMask(1 << RenderWorkManager::Scene);
			pInst->SetParameter("AlbedoRoughness", Vector4f(0.0f, 1.0f, 0.0f, 0.1f));
			pInst->SetParameter("AOMetalic", Vector2f(1.0f, 1.0f));
			pInst->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
			pInst->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
			pInst->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");
		}

		m_innerBallMaterialInstances.push_back(pInst);
	}

	m_pQuadMaterialInstance = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pQuadMaterialInstance->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pQuadMaterialInstance->SetParameter("AlbedoRoughness", Vector4f(0.7f, 0.7f, 0.7f, 0.1f));
	m_pQuadMaterialInstance->SetParameter("AOMetalic", Vector2f(1.0f, 0.99f));
	m_pQuadMaterialInstance->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, "AluminumAlbedoRoughness");
	m_pQuadMaterialInstance->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, "AluminumNormalAO");
	m_pQuadMaterialInstance->SetMaterialTexture("MetallicTextureIndex", R8_1024, "AluminumMetalic");

	m_pBoxMaterialInstance0 = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pBoxMaterialInstance0->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pBoxMaterialInstance0->SetParameter("AlbedoRoughness", Vector4f(1.0f, 1.0f, 1.0f, 0.9f));
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

	m_pBoxMaterialInstance3 = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pBoxMaterialInstance3->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pBoxMaterialInstance3->SetParameter("AlbedoRoughness", Vector4f(1.0f, 1.0f, 0.0f, 0.9f));
	m_pBoxMaterialInstance3->SetParameter("AOMetalic", Vector2f(1.0f, 0.1f));
	m_pBoxMaterialInstance3->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
	m_pBoxMaterialInstance3->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	m_pBoxMaterialInstance3->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_pBoxMaterialInstance4 = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	m_pBoxMaterialInstance4->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pBoxMaterialInstance4->SetParameter("AlbedoRoughness", Vector4f(1.0f, 0.0f, 0.0f, 0.9f));
	m_pBoxMaterialInstance4->SetParameter("AOMetalic", Vector2f(1.0f, 0.1f));
	m_pBoxMaterialInstance4->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
	m_pBoxMaterialInstance4->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	m_pBoxMaterialInstance4->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_pSophiaMaterialInstance = RenderWorkManager::GetInstance()->AcquirePBRSkinnedMaterialInstance();
	m_pSophiaMaterialInstance->SetRenderMask(1 << RenderWorkManager::Scene);
	m_pSophiaMaterialInstance->SetParameter("AlbedoRoughness", Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
	m_pSophiaMaterialInstance->SetParameter("AOMetalic", Vector2f(1.0f, 0.0f));
	m_pSophiaMaterialInstance->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, "SophiaAlbedoRoughness");
	m_pSophiaMaterialInstance->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, "SophiaNormalAO");
	m_pSophiaMaterialInstance->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_pPlanetMaterialInstance = RenderWorkManager::GetInstance()->AcquirePBRPlanetMaterialInstance();

	m_pShadowMapMaterialInstance = RenderWorkManager::GetInstance()->AcquireShadowMaterialInstance();
	m_pSkinnedShadowMapMaterialInstance = RenderWorkManager::GetInstance()->AcquireSkinnedShadowMaterialInstance();
}

void VulkanGlobal::AddBoneBox(const std::shared_ptr<BaseObject>& pObject)
{
	std::shared_ptr<MaterialInstance> pInst = RenderWorkManager::GetInstance()->AcquirePBRMaterialInstance();
	pInst->SetRenderMask(1 << RenderWorkManager::Scene);
	pInst->SetParameter("AlbedoRoughness", Vector4f(1.0f, 1.0f, 0.0f, 0.5f));
	pInst->SetParameter("AOMetalic", Vector2f(1.0f, 0.9f));
	pInst->SetMaterialTexture("AlbedoRoughnessTextureIndex", RGBA8_1024, ":)");
	pInst->SetMaterialTexture("NormalAOTextureIndex", RGBA8_1024, ":)");
	pInst->SetMaterialTexture("MetallicTextureIndex", R8_1024, ":)");

	m_boneBoxMaterialInstances.push_back(pInst);
	m_boneBoxRenderers.push_back(MeshRenderer::Create(m_pPBRBoxMesh, pInst));

	pObject->AddComponent(m_boneBoxRenderers.back());

	for (uint32_t i = 0; i < pObject->GetChildrenCount(); i++)
	{
		AddBoneBox(pObject->GetChild(i));
	}
}

void VulkanGlobal::InitScene()
{
	UniformData::GetInstance()->GetPerFrameUniforms()->SetMainLightColor({ 1, 1, 1 });
	UniformData::GetInstance()->GetPerFrameUniforms()->SetMainLightDir({ 1, 1, -1 });

	UniformData::GetInstance()->GetGlobalUniforms()->SetRenderSettings({ 1.0 / 2.2, 4.5, 11.2, 0.0 });

	UniformData::GetInstance()->GetGlobalUniforms()->SetBRDFBias(0.7);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSSRMip(1.0);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSampleNormalRegenCount(15.0);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSampleNormalRegenMargin(0.19);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSSRTStride(3.7);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSSRTInitOffset(2.0);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMaxSSRTStepCount(200.0);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSSRTThickness(0.05);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSSRTBorderFadingDist(0.05);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSSRTStepCountFadingDist(0.1);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSSRTMaxDistance(5000000);

	uint32_t smaller = FrameBufferDiction::WINDOW_HEIGHT < FrameBufferDiction::WINDOW_WIDTH ? FrameBufferDiction::WINDOW_HEIGHT : FrameBufferDiction::WINDOW_WIDTH;
	UniformData::GetInstance()->GetGlobalUniforms()->SetScreenSizeMipLevel(log2(smaller) + 1);

	UniformData::GetInstance()->GetGlobalUniforms()->SetMotionImpactLowerBound(0.0001);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMotionImpactUpperBound(0.003);
	UniformData::GetInstance()->GetGlobalUniforms()->SetHighResponseSSRPortion(0.7);

	UniformData::GetInstance()->GetGlobalUniforms()->SetBloomClampingLowerBound(1);
	UniformData::GetInstance()->GetGlobalUniforms()->SetBloomClampingUpperBound(5);
	UniformData::GetInstance()->GetGlobalUniforms()->SetUpsampleScale(1.0);
	UniformData::GetInstance()->GetGlobalUniforms()->SetBloomAmplify(1.0);
	UniformData::GetInstance()->GetGlobalUniforms()->SetBloomSlope(1.0);

	UniformData::GetInstance()->GetGlobalUniforms()->SetMaxCOC(16.0 / FrameBufferDiction::WINDOW_WIDTH);

	UniformData::GetInstance()->GetGlobalUniforms()->SetMotionBlurAmplify(0.06);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMotionBlurSampleCount(16);

	UniformData::GetInstance()->GetGlobalUniforms()->SetVignetteMinDist(0.2);
	UniformData::GetInstance()->GetGlobalUniforms()->SetVignetteMaxDist(0.8);
	UniformData::GetInstance()->GetGlobalUniforms()->SetVignetteAmplify(0.7);

	UniformData::GetInstance()->GetGlobalUniforms()->SetSSAOSampleCount(32);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSSAOSampleRadius(0.3);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSSAOScreenSpaceSampleLength(1.0 / FrameBufferDiction::WINDOW_WIDTH * 100);
	UniformData::GetInstance()->GetGlobalUniforms()->SetSSAOCurveFactor(0.3);

	// Render normalized spherical planet at the height of 0.001 * planet radius
	// For earth it should be higher than 6km
	UniformData::GetInstance()->GetGlobalUniforms()->SetPlanetSphericalTransitionRatio(0.001);
	UniformData::GetInstance()->GetGlobalUniforms()->SetPlanetTriangleScreenSize(400);
	UniformData::GetInstance()->GetGlobalUniforms()->SetMaxPlanetLODLevel(32);
	UniformData::GetInstance()->GetGlobalUniforms()->SetPlanetPatchSubdivideCount(std::pow(2, m_LODPatchLevel));

	UniformData::GetInstance()->GetGlobalUniforms()->SetPlanetEdgeRenderFactor(1);
	UniformData::GetInstance()->GetGlobalUniforms()->SetPlanetPatchEdgeWidth(0.01);
	UniformData::GetInstance()->GetGlobalUniforms()->SetPlanetTriangleEdgeWidth(0.03);
	UniformData::GetInstance()->GetGlobalUniforms()->SetPlanetMorphingRange(0.25f);

	UniformData::GetInstance()->GetGlobalUniforms()->SetSunSizeInCosine(std::cos(0.00935 / 2.0));

	PhysicalCamera::PhysicalCameraProps props =
	{
		1440.0f / 1024.0f,
		0.035f,
		0.035f,
		1,
		3.5f,
		1 / 500.0f,
		100.0f,
		2000.0f
	};

	m_pCameraComp = PhysicalCamera::Create(props);
	m_pCameraObj = BaseObject::Create();
	m_pCameraObj->AddComponent(m_pCameraComp);

	m_pCharacter = Character::Create({ 0.002f, 0.005f });
	m_pCameraObj->AddComponent(m_pCharacter);
	m_pCameraObj->AddComponent(FrustumJitter::Create());

	m_pCameraObj->SetPos({ 0, 1, -2.2f });
	m_pCameraObj->SetRotation(Matrix3d::EulerAngle(-0.78f, 3.14f, 0));

	m_pDirLightObj = BaseObject::Create();
	m_pDirLightObj->SetPos(0.64f, 0.64f, -0.64f);
	m_pDirLightObj->SetRotation(Matrix3d::EulerAngle(0.78f, 0, 0) * Matrix3d::EulerAngle(0, 2.355f, 0));

	m_pDirLight = DirectionLight::Create({ 4.0f, 4.0f, 4.0f });
	m_pDirLightObj->AddComponent(m_pDirLight);

	m_pSphere1 = BaseObject::Create();
	m_pSphere2 = BaseObject::Create();
	m_pSphere3 = BaseObject::Create();

	m_pQuadObject = BaseObject::Create();
	m_pBoxObject0 = BaseObject::Create();
	m_pBoxObject1 = BaseObject::Create();
	m_pBoxObject2 = BaseObject::Create();
	m_pBoxObject3 = BaseObject::Create();
	m_pBoxObject4 = BaseObject::Create();

	m_pQuadRenderer = MeshRenderer::Create(m_pQuadMesh, { m_pQuadMaterialInstance, m_pShadowMapMaterialInstance });
	m_pBoxRenderer0 = MeshRenderer::Create(m_pPBRBoxMesh, { m_pBoxMaterialInstance0, m_pShadowMapMaterialInstance });
	m_pBoxRenderer1 = MeshRenderer::Create(m_pPBRBoxMesh, { m_pBoxMaterialInstance1, m_pShadowMapMaterialInstance });
	m_pBoxRenderer2 = MeshRenderer::Create(m_pPBRBoxMesh, { m_pBoxMaterialInstance2, m_pShadowMapMaterialInstance });
	m_pBoxRenderer3 = MeshRenderer::Create(m_pPBRBoxMesh, { m_pBoxMaterialInstance3 });
	m_pBoxRenderer4 = MeshRenderer::Create(m_pPBRBoxMesh, { m_pBoxMaterialInstance4 });

	m_pPlanetGenerator = PlanetGenerator::Create(m_pCameraComp, 6360000);

	AssimpSceneReader::SceneInfo sceneInfo;

	m_pGunObject = AssimpSceneReader::ReadAndAssemblyScene("../data/textures/cerberus/cerberus.fbx", { VertexFormatPNTCT }, sceneInfo);
	m_pGunMesh = sceneInfo.meshLinks[0].first;
	m_pGunMeshRenderer = MeshRenderer::Create(m_pGunMesh, { m_pGunMaterialInstance, m_pShadowMapMaterialInstance });
	sceneInfo.meshLinks[0].second->AddComponent(m_pGunMeshRenderer);
	sceneInfo.meshLinks.clear();
	m_pGunObject->SetPos({ -0.8f, -0.08f, 0 });
	m_pGunObject->SetScale(0.01f);

	m_pSphere0 = AssimpSceneReader::ReadAndAssemblyScene("../data/models/sphere.obj", { VertexFormatPNTCT }, sceneInfo);
	m_pSphereRenderer0 = MeshRenderer::Create(sceneInfo.meshLinks[0].first, { m_pSphereMaterialInstance0, m_pShadowMapMaterialInstance });
	sceneInfo.meshLinks[0].second->AddComponent(m_pSphereRenderer0);
	m_pSphere0->SetPos(0.4f, -0.15f, 0);
	m_pSphere0->SetScale(0.01f);

	m_pSphereRenderer1 = MeshRenderer::Create(sceneInfo.meshLinks[0].first, { m_pSphereMaterialInstance1, m_pShadowMapMaterialInstance });
	m_pSphere1->AddComponent(m_pSphereRenderer1);
	m_pSphere1->SetPos(1, -0.15f, 0);
	m_pSphere1->SetScale(0.01f);

	m_pSphereRenderer2 = MeshRenderer::Create(sceneInfo.meshLinks[0].first, { m_pSphereMaterialInstance2, m_pShadowMapMaterialInstance });
	m_pSphere2->AddComponent(m_pSphereRenderer2);
	m_pSphere2->SetPos(1, -0.15f, 0.6f);
	m_pSphere2->SetScale(0.01f);

	m_pSphereRenderer3 = MeshRenderer::Create(sceneInfo.meshLinks[0].first, { m_pSphereMaterialInstance3 });
	m_pSphere3->AddComponent(m_pSphereRenderer3);
	m_pSphere3->SetPos(150000, 5000, 150000);
	m_pSphere3->SetScale(1000.0);
	sceneInfo.meshLinks.clear();

	m_pInnerBall = AssimpSceneReader::ReadAndAssemblyScene("../data/models/Sample.FBX", { VertexFormatPNTCT }, sceneInfo);
	for (uint32_t i = 0; i < sceneInfo.meshLinks.size(); i++)
	{
		m_innerBallRenderers.push_back(MeshRenderer::Create(sceneInfo.meshLinks[i].first, { m_innerBallMaterialInstances[i], m_pShadowMapMaterialInstance }));
		sceneInfo.meshLinks[i].second->AddComponent(m_innerBallRenderers[i]);
	}
	m_pInnerBall->SetPos(-1.3f, -0.4f, 0);
	m_pInnerBall->SetRotation(Quaterniond(Vector3d(0, 1, 0), 3.14));
	m_pInnerBall->SetScale(0.005f);
	sceneInfo.meshLinks.clear();

	m_pQuadObject->AddComponent(m_pQuadRenderer);
	m_pQuadObject->SetPos(-0.5f, -0.4f, 0);
	m_pQuadObject->SetScale(2);

	m_pBoxObject0->AddComponent(m_pBoxRenderer0);
	m_pBoxObject0->SetScale(0.3f);
	m_pBoxObject0->SetPos(-0.2f, -0.1f, 0);

	m_pBoxObject1->AddComponent(m_pBoxRenderer1);
	m_pBoxObject1->SetScale(0.15f);
	m_pBoxObject1->SetPos(-0.2f, 0.35f, 0);

	m_pBoxObject2->AddComponent(m_pBoxRenderer2);
	m_pBoxObject2->SetScale(0.15f);
	m_pBoxObject2->SetPos(-0.2f, -0.25f, 0.5f);

	m_pBoxObject3->AddComponent(m_pBoxRenderer3);
	m_pBoxObject3->SetScale({ 700000, 1000, 1000 });
	m_pBoxObject3->SetPos(0, -9000, 7000);
	m_pBoxObject3->SetRotation(Matrix3d::EulerAngle(0, -0.61, 0));

	m_pBoxObject4->AddComponent(m_pBoxRenderer4);
	m_pBoxObject4->SetScale({ 10000, 700000, 10000 });
	m_pBoxObject4->SetPos(130000, 5000, 150000);
	m_pBoxObject4->SetRotation(Matrix3d::EulerAngle(0, -0.5, 0));

	Quaterniond rot = Quaterniond(Vector3d(1, 0, 0), 0);
	m_pQuadObject->SetRotation(Quaterniond(Vector3d(1, 0, 0), -1.57));

	m_pSophiaObject = AssimpSceneReader::ReadAndAssemblyScene("../data/models/rp_sophia_animated_003_idling.FBX", { VertexFormatPNTCTB }, sceneInfo);
	m_pSophiaMesh = sceneInfo.meshLinks[0].first;

	std::shared_ptr<AnimationController> pAnimationController = m_pSophiaObject->GetComponent<AnimationController>();
	m_pSophiaRenderer = MeshRenderer::Create(m_pSophiaMesh, { m_pSophiaMaterialInstance, m_pSkinnedShadowMapMaterialInstance });
	pAnimationController->SetMeshRenderer(m_pSophiaRenderer);
	sceneInfo.meshLinks[0].second->AddComponent(m_pSophiaRenderer);
	m_pSophiaRenderer->SetName(L"hehe");
	m_pSophiaObject->SetScale(0.005f);
	m_pSophiaObject->SetRotation(Quaterniond(Vector3d(0, 1, 0), 3.14f));
	m_pSophiaObject->SetPos(0, -0.4f, -1);
	//AddBoneBox(m_pSophiaObject);
	sceneInfo.meshLinks.clear();

	m_pPlanetRenderer = MeshRenderer::Create(m_pLODTriangleMesh, m_pPlanetMaterialInstance);

	m_pPlanetObject = BaseObject::Create();
	m_pPlanetObject->AddComponent(m_pPlanetGenerator);
	m_pPlanetObject->AddComponent(m_pPlanetRenderer);


	m_pSceneRootObject = BaseObject::Create();
	m_pSceneRootObject->AddChild(m_pCameraObj);
	m_pSceneRootObject->AddChild(m_pGunObject);
	m_pSceneRootObject->AddChild(m_pSphere0);
	m_pSceneRootObject->AddChild(m_pSphere1);
	m_pSceneRootObject->AddChild(m_pSphere2);
	m_pSceneRootObject->AddChild(m_pSphere3);
	m_pSceneRootObject->AddChild(m_pInnerBall);
	m_pSceneRootObject->AddChild(m_pQuadObject);
	m_pSceneRootObject->AddChild(m_pBoxObject0);
	m_pSceneRootObject->AddChild(m_pBoxObject1);
	m_pSceneRootObject->AddChild(m_pBoxObject2);
	m_pSceneRootObject->AddChild(m_pBoxObject3);
	m_pSceneRootObject->AddChild(m_pBoxObject4);
	m_pSceneRootObject->AddChild(m_pSophiaObject);
	m_pSceneRootObject->AddChild(m_pDirLightObj);
	m_pSceneRootObject->SetPosY(m_pPlanetGenerator->GetPlanetRadius() + 9000);

	m_pRootObject = BaseObject::Create();
	m_pRootObject->AddChild(m_pSceneRootObject);
	m_pRootObject->AddChild(m_pPlanetObject);
}

class VariableChanger : public IInputListener
{
public:
	void ProcessKey(KeyState keyState, uint8_t keyCode) override;
	void ProcessMouse(KeyState keyState, const Vector2d& mousePosition) override {}
	void ProcessMouse(const Vector2d& mousePosition) override {}
	float var = 0.0;
	bool boolVar = true;
};

void VariableChanger::ProcessKey(KeyState keyState, uint8_t keyCode)
{
	static float interval = 0.01f;
	static float varInterval = 0.005f;
	if (keyCode == KEY_F)
	{
		var += varInterval;
		var = var > 1.0f ? 1.0f : var;
	}
	if (keyCode == KEY_L)
	{
		var -= varInterval;
		var = var < 0 ? 0 : var;
	}
	if (keyCode == KEY_T && keyState == KEY_UP)
	{
		boolVar = !boolVar;
	}
}

std::shared_ptr<VariableChanger> c;

void VulkanGlobal::EndSetup()
{
	GlobalDeviceObjects::GetInstance()->GetStagingBufferMgr()->FlushDataMainThread();
	m_commandBufferList.resize(GetSwapChain()->GetSwapChainImageCount() * 2);

	m_pRootObject->Awake();
	m_pRootObject->Start();

	c = std::make_shared<VariableChanger>();
	InputHub::GetInstance()->Register(c);
}

void VulkanGlobal::Draw()
{
	static uint32_t pingpong = 0;
	static uint32_t nextPingpong = 1;
	static uint32_t frameCount = 0;

	GetSwapChain()->AcquireNextImage();

	uint32_t frameIndex = FrameMgr()->FrameIndex();
	uint32_t cbIndex = frameIndex * 2 + pingpong;
	nextPingpong = (pingpong + 1) % 2;

	FrameEventManager::GetInstance()->OnFrameBegin();

	UniformData::GetInstance()->GetPerFrameUniforms()->SetDeltaTime(Timer::GetElapsedTime());
	UniformData::GetInstance()->GetPerFrameUniforms()->SetSinTime(std::sin(Timer::GetTotalTime()));
	UniformData::GetInstance()->GetPerFrameUniforms()->SetFrameIndex(frameIndex);
	UniformData::GetInstance()->GetPerFrameUniforms()->SetPingpongIndex(nextPingpong);
	UniformData::GetInstance()->GetPerFrameUniforms()->SetHaltonIndexX8Jitter(HaltonSequence::GetHaltonJitter(HaltonSequence::x8, frameCount));
	UniformData::GetInstance()->GetPerFrameUniforms()->SetHaltonIndexX16Jitter(HaltonSequence::GetHaltonJitter(HaltonSequence::x16, frameCount));
	UniformData::GetInstance()->GetPerFrameUniforms()->SetHaltonIndexX32Jitter(HaltonSequence::GetHaltonJitter(HaltonSequence::x32, frameCount));
	UniformData::GetInstance()->GetPerFrameUniforms()->SetHaltonIndexX256Jitter(HaltonSequence::GetHaltonJitter(HaltonSequence::x256, frameCount));

	RenderWorkManager::GetInstance()->SetRenderStateMask((1 << RenderWorkManager::Scene) | (1 << RenderWorkManager::ShadowMapGen));

	m_pCameraComp->SetFocalLength((1.0f - c->var) * 0.035f + c->var * 0.2f);
	m_pPlanetGenerator->ToggleCameraInfoUpdate(c->boolVar);

	m_pRootObject->Update();
	m_pRootObject->OnAnimationUpdate();
	m_pRootObject->LateUpdate();
	m_pRootObject->UpdateCachedData();
	m_pRootObject->OnPreRender();
	m_pRootObject->OnRenderObject();

	// Sync data for current frame before rendering
	UniformData::GetInstance()->SyncDataBuffer();
	RenderWorkManager::GetInstance()->SyncMaterialData();
	PerFrameData::GetInstance()->SyncDataBuffer();

	RenderWorkManager::GetInstance()->OnFrameBegin();

	static bool newCBCreated = false;
	if (!PREBAKE_CB)
	{
		m_commandBufferList[cbIndex] = m_perFrameRes[FrameMgr()->FrameIndex()]->AllocateTransientPrimaryCommandBuffer();
		newCBCreated = true;
	}
	else if (m_commandBufferList[cbIndex] == nullptr)
	{
		m_commandBufferList[cbIndex] = m_perFrameRes[FrameMgr()->FrameIndex()]->AllocatePersistantPrimaryCommandBuffer();
		newCBCreated = true;
	}

	if (newCBCreated)
	{
		m_commandBufferList[cbIndex]->StartPrimaryRecording();

		RenderWorkManager::GetInstance()->Draw(m_commandBufferList[cbIndex], pingpong);

		m_commandBufferList[cbIndex]->EndPrimaryRecording();

		newCBCreated = false;
	}

	m_pRootObject->OnPostRender();

	RenderWorkManager::GetInstance()->OnFrameEnd();

	FrameMgr()->CacheSubmissioninfo(GlobalGraphicQueue(), { m_commandBufferList[cbIndex] }, {}, false);
	
	GetSwapChain()->QueuePresentImage(GlobalObjects()->GetPresentQueue());

	pingpong = nextPingpong;
	frameCount++;

	FrameEventManager::GetInstance()->OnFrameEnd();
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