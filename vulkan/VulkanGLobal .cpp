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
#include "RenderWorkManager.h"
#include <iostream>
#include "GlobalVulkanStates.h"

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
	m_pEnvFrameBuffer = FrameBuffer::CreateOffScreenFrameBuffer(m_pDevice, OffScreenSize, OffScreenSize, RenderWorkManager::GetDefaultOffscreenRenderPass());

	m_offscreenFrameBuffers.resize(GlobalDeviceObjects::GetInstance()->GetSwapChain()->GetSwapChainImageCount());
	for (uint32_t i = 0; i < m_offscreenFrameBuffers.size(); i++)
		m_offscreenFrameBuffers[i] = FrameBuffer::CreateOffScreenFrameBuffer(m_pDevice, GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, GetPhysicalDevice()->GetSurfaceCap().currentExtent.height, RenderWorkManager::GetDefaultOffscreenRenderPass());
}

void VulkanGlobal::InitVertices()
{
	m_pGunMesh = Mesh::Create("../data/textures/cerberus/cerberus.fbx");

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
		cubeVertices, 8, 1 << Mesh::VAFPosition,
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
		quadVertices, 4, (1 << Mesh::VAFPosition) | (1 << Mesh::VAFTexCoord), 
		quadIndices, 6, VK_INDEX_TYPE_UINT32
	);
}

void VulkanGlobal::InitUniforms()
{
	uint32_t minAlign = GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	uint32_t alignedBytes = sizeof(GlobalUniforms) / minAlign * minAlign + (sizeof(GlobalUniforms) % minAlign > 0 ? minAlign : 0);
	uint32_t totalUniformBytes = alignedBytes * GetSwapChain()->GetSwapChainImageCount();
	m_pUniformBuffer = UniformBuffer::Create(m_pDevice, totalUniformBytes);

	m_pAlbedo = Texture2D::Create(m_pDevice, "../data/textures/cerberus/albedo.ktx", VK_FORMAT_R8G8B8A8_UNORM);
	m_pAmbientOcclusion = Texture2D::Create(m_pDevice, "../data/textures/cerberus/ao.ktx", VK_FORMAT_R8_UNORM);
	m_pMetalic = Texture2D::Create(m_pDevice, "../data/textures/cerberus/metallic.ktx", VK_FORMAT_R8_UNORM);
	m_pNormal = Texture2D::Create(m_pDevice, "../data/textures/cerberus/normal.ktx", VK_FORMAT_R8G8B8A8_UNORM);
	m_pRoughness = Texture2D::Create(m_pDevice, "../data/textures/cerberus/roughness.ktx", VK_FORMAT_R8_UNORM);
	m_pSkyBoxTex = TextureCube::Create(m_pDevice, "../data/textures/hdr/gcanyon_cube.ktx", VK_FORMAT_R16G16B16A16_SFLOAT);
	//m_pSimpleTex = Texture2D::Create(m_pDevice, "../data/textures/cerberus/albedo.ktx", VK_FORMAT_R8G8B8A8_UNORM);
	m_pSimpleTex = Texture2D::CreateEmptyTexture(m_pDevice, OffScreenSize, OffScreenSize, VK_FORMAT_R16G16B16A16_SFLOAT);
	m_pIrradianceTex = TextureCube::CreateEmptyTextureCube(m_pDevice, OffScreenSize, OffScreenSize, VK_FORMAT_R16G16B16A16_SFLOAT);
	m_pPrefilterEnvTex = TextureCube::CreateEmptyTextureCube(m_pDevice, OffScreenSize, OffScreenSize, std::log2(OffScreenSize) + 1, VK_FORMAT_R16G16B16A16_SFLOAT);

	// FIXME: 2 channels should be enough, I don't intend to do it now as I have to create new render passes and frame buffers, leave it to later refactor
	m_pBRDFLut = Texture2D::CreateEmptyTexture(m_pDevice, OffScreenSize, OffScreenSize, VK_FORMAT_R16G16B16A16_SFLOAT);
}

void VulkanGlobal::InitIrradianceMap()
{
	GetGlobalVulkanStates()->SetRenderState(GlobalVulkanStates::IrradianceGen);

	Vector3f up = { 0, 1, 0 };
	Vector3f look = { 0, 0, -1 };
	look.Normalize();
	Vector3f xaxis = up ^ look.Negativate();
	xaxis.Normalize();
	Vector3f yaxis = look ^ xaxis;
	yaxis.Normalize();

	Matrix3f rotation;
	rotation.c[0] = xaxis;
	rotation.c[1] = yaxis;
	rotation.c[2] = look;

	Matrix3f cameraRotations[] =
	{
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * Matrix3f::Rotation(3.0 * 3.14159265 / 2.0, Vector3f(0, 1, 0)) * rotation,	// Positive X, i.e right
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * Matrix3f::Rotation(1.0 * 3.14159265 / 2.0, Vector3f(0, 1, 0)) * rotation,	// Negative X, i.e left
		Matrix3f::Rotation(3.0 * 3.14159265 / 2.0, Vector3f(1, 0, 0)) * rotation,	// Positive Y, i.e top
		Matrix3f::Rotation(1.0 * 3.14159265 / 2.0, Vector3f(1, 0, 0)) * rotation,	// Negative Y, i.e bottom
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * rotation,	// Positive Z, i.e back
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(0, 0, 1)) * rotation,	// Negative Z, i.e front
	};

	for (uint32_t i = 0; i < 6; i++)
	{
		std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();

		std::vector<VkClearValue> clearValues =
		{
			{ 0.2f, 0.2f, 0.2f, 0.2f },
			{ 1.0f, 0 }
		};

		VkViewport viewport =
		{
			0, 0,
			OffScreenSize, OffScreenSize,
			0, 1
		};

		VkRect2D scissorRect =
		{
			0, 0,
			OffScreenSize, OffScreenSize,
		};

		GetGlobalVulkanStates()->SetViewport(viewport);
		GetGlobalVulkanStates()->SetScissorRect(scissorRect);

		m_pOffScreenCamObj->SetRotation(cameraRotations[i]);

		pDrawCmdBuffer->StartPrimaryRecording();

		UpdateUniforms(0, m_pOffScreenCamComp);
		StagingBufferMgr()->RecordDataFlush(pDrawCmdBuffer);

		uint32_t offset = 0;
		
		pDrawCmdBuffer->BeginRenderPass(clearValues, true);

		m_pRootObject->Update();
		RenderWorkMgr()->GetCurrentRenderPass()->ExecuteCachedSecondaryCommandBuffers(pDrawCmdBuffer);

		pDrawCmdBuffer->EndRenderPass();


		pDrawCmdBuffer->EndPrimaryRecording();

		GlobalGraphicQueue()->SubmitCommandBuffer(pDrawCmdBuffer, nullptr, true);

		m_pEnvFrameBuffer->ExtractContent(m_pIrradianceTex, 0, 1, i, 1);
	}
}

void VulkanGlobal::InitPrefilterEnvMap()
{
	GetGlobalVulkanStates()->SetRenderState(GlobalVulkanStates::ReflectionGen);

	Vector3f up = { 0, 1, 0 };
	Vector3f look = { 0, 0, -1 };
	look.Normalize();
	Vector3f xaxis = up ^ look.Negativate();
	xaxis.Normalize();
	Vector3f yaxis = look ^ xaxis;
	yaxis.Normalize();

	Matrix3f rotation;
	rotation.c[0] = xaxis;
	rotation.c[1] = yaxis;
	rotation.c[2] = look;

	Matrix3f cameraRotations[] =
	{
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * Matrix3f::Rotation(3.0 * 3.14159265 / 2.0, Vector3f(0, 1, 0)) * rotation,	// Positive X, i.e right
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * Matrix3f::Rotation(1.0 * 3.14159265 / 2.0, Vector3f(0, 1, 0)) * rotation,	// Negative X, i.e left
		Matrix3f::Rotation(3.0 * 3.14159265 / 2.0, Vector3f(1, 0, 0)) * rotation,	// Positive Y, i.e top
		Matrix3f::Rotation(1.0 * 3.14159265 / 2.0, Vector3f(1, 0, 0)) * rotation,	// Negative Y, i.e bottom
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * rotation,	// Positive Z, i.e back
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(0, 0, 1)) * rotation,	// Negative Z, i.e front
	};

	uint32_t mipLevels = std::log2(OffScreenSize);
	for (uint32_t mipLevel = 0; mipLevel < mipLevels + 1; mipLevel++)
	{
		m_globalUniforms.roughness = mipLevel / (float)mipLevels;
		uint32_t size = std::pow(2, mipLevels - mipLevel);
		for (uint32_t i = 0; i < 6; i++)
		{
			std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();

			std::vector<VkClearValue> clearValues =
			{
				{ 0.2f, 0.2f, 0.2f, 0.2f },
				{ 1.0f, 0 }
			};

			VkViewport viewport =
			{
				0, 0,
				size, size,
				0, 1
			};

			VkRect2D scissorRect =
			{
				0, 0,
				size, size,
			};

			GetGlobalVulkanStates()->SetViewport(viewport);
			GetGlobalVulkanStates()->SetScissorRect(scissorRect);

			m_pOffScreenCamObj->SetRotation(cameraRotations[i]);

			pDrawCmdBuffer->StartPrimaryRecording();

			UpdateUniforms(0, m_pOffScreenCamComp);
			StagingBufferMgr()->RecordDataFlush(pDrawCmdBuffer);

			uint32_t offset = 0;

			pDrawCmdBuffer->BeginRenderPass(clearValues, true);

			m_pRootObject->Update();

			RenderWorkMgr()->GetCurrentRenderPass()->ExecuteCachedSecondaryCommandBuffers(pDrawCmdBuffer);


			pDrawCmdBuffer->EndRenderPass();


			pDrawCmdBuffer->EndPrimaryRecording();

			GlobalGraphicQueue()->SubmitCommandBuffer(pDrawCmdBuffer, nullptr, true);

			m_pEnvFrameBuffer->ExtractContent(m_pPrefilterEnvTex, mipLevel, 1, i, 1, size, size);
		}
	}
}

void VulkanGlobal::InitBRDFlutMap()
{
	GetGlobalVulkanStates()->SetRenderState(GlobalVulkanStates::BrdfLutGen);

	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();

	std::vector<VkClearValue> clearValues =
	{
		{ 0.2f, 0.2f, 0.2f, 0.2f },
		{ 1.0f, 0 }
	};

	VkViewport viewport =
	{
		0, 0,
		OffScreenSize, OffScreenSize,
		0, 1
	};

	VkRect2D scissorRect =
	{
		0, 0,
		OffScreenSize, OffScreenSize,
	};

	pDrawCmdBuffer->StartPrimaryRecording();

	GetGlobalVulkanStates()->SetViewport(viewport);
	GetGlobalVulkanStates()->SetScissorRect(scissorRect);

	uint32_t offset = 0;

	pDrawCmdBuffer->BeginRenderPass(clearValues, true);

	m_pRootObject->Update();

	RenderWorkMgr()->GetCurrentRenderPass()->ExecuteCachedSecondaryCommandBuffers(pDrawCmdBuffer);

	pDrawCmdBuffer->EndRenderPass();


	pDrawCmdBuffer->EndPrimaryRecording();

	GlobalGraphicQueue()->SubmitCommandBuffer(pDrawCmdBuffer, nullptr, true);

	m_pEnvFrameBuffer->ExtractContent(m_pBRDFLut);
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
	std::vector<VkDescriptorSetLayoutBinding> dsLayoutBindings =
	{
		{
			0,	//binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	//type
			1,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			1,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			2,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			3,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			4,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			5,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			6,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			7,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			8,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		}
	};

	SimpleMaterialCreateInfo info =
	{
		{ L"../data/shaders/pbr.vert.spv", L"", L"", L"", L"../data/shaders/pbr.frag.spv", L"" },
		{ dsLayoutBindings },
		{ m_pGunMesh->GetVertexBuffer()->GetBindingDesc() },
		m_pGunMesh->GetVertexBuffer()->GetAttribDesc(),
		RenderWorkManager::GetDefaultRenderPass()
	};

	m_pGunMaterial = Material::CreateDefaultMaterial(info);
	m_pGunMaterialInstance = m_pGunMaterial->CreateMaterialInstance();
	m_pGunMaterialInstance->SetRenderMask(1 << GlobalVulkanStates::Scene);
	m_pGunMaterialInstance->GetDescriptorSet(0)->UpdateBufferDynamic(0, m_pUniformBuffer);
	m_pGunMaterialInstance->GetDescriptorSet(0)->UpdateImage(1, m_pAlbedo);
	m_pGunMaterialInstance->GetDescriptorSet(0)->UpdateImage(2, m_pNormal);
	m_pGunMaterialInstance->GetDescriptorSet(0)->UpdateImage(3, m_pRoughness);
	m_pGunMaterialInstance->GetDescriptorSet(0)->UpdateImage(4, m_pMetalic);
	m_pGunMaterialInstance->GetDescriptorSet(0)->UpdateImage(5, m_pAmbientOcclusion);
	m_pGunMaterialInstance->GetDescriptorSet(0)->UpdateImage(6, m_pIrradianceTex);
	m_pGunMaterialInstance->GetDescriptorSet(0)->UpdateImage(7, m_pPrefilterEnvTex);
	m_pGunMaterialInstance->GetDescriptorSet(0)->UpdateImage(8, m_pBRDFLut);

	// Skybox material
	dsLayoutBindings =
	{
		{
			0,	//binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	//type
			1,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			1,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		}
	};

	info =
	{
		{ L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/sky_box.frag.spv", L"" },
		{ dsLayoutBindings },
		{ m_pCubeMesh->GetVertexBuffer()->GetBindingDesc() },
		m_pCubeMesh->GetVertexBuffer()->GetAttribDesc(),
		RenderWorkManager::GetDefaultRenderPass()
	};
	m_pSkyBoxMaterial = Material::CreateDefaultMaterial(info);
	m_pSkyBoxMaterialInstance = m_pSkyBoxMaterial->CreateMaterialInstance();
	m_pSkyBoxMaterialInstance->SetRenderMask(1 << GlobalVulkanStates::Scene);
	m_pSkyBoxMaterialInstance->GetDescriptorSet(0)->UpdateBufferDynamic(0, m_pUniformBuffer);
	m_pSkyBoxMaterialInstance->GetDescriptorSet(0)->UpdateImage(1, m_pSkyBoxTex);

	info =
	{
		{ L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/irradiance.frag.spv", L"" },
		{ dsLayoutBindings },
		{ m_pCubeMesh->GetVertexBuffer()->GetBindingDesc() },
		m_pCubeMesh->GetVertexBuffer()->GetAttribDesc(),
		RenderWorkManager::GetDefaultOffscreenRenderPass()
	};

	m_pSkyBoxIrradianceMaterial = Material::CreateDefaultMaterial(info);
	m_pSkyBoxIrradianceMaterialInstance = m_pSkyBoxIrradianceMaterial->CreateMaterialInstance();
	m_pSkyBoxIrradianceMaterialInstance->SetRenderMask(1 << GlobalVulkanStates::IrradianceGen);
	m_pSkyBoxIrradianceMaterialInstance->GetDescriptorSet(0)->UpdateBufferDynamic(0, m_pUniformBuffer);
	m_pSkyBoxIrradianceMaterialInstance->GetDescriptorSet(0)->UpdateImage(1, m_pSkyBoxTex);

	/*
	info = {};
	info.pRenderPass = RenderWorkManager::GetDefaultOffscreenRenderPass();
	info.pPipelineLayout = m_pSkyBoxPLayout;
	info.pVertShader = m_pSkyBoxVS;
	info.pFragShader = m_pPrefilterEnvFS;
	info.vertexBindingsInfo = { m_pCubeMesh->GetVertexBuffer()->GetBindingDesc() };
	info.vertexAttributesInfo = m_pCubeMesh->GetVertexBuffer()->GetAttribDesc();
	*/

	info =
	{
		{ L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/prefilter_env.frag.spv", L"" },
		{ dsLayoutBindings },
		{ m_pCubeMesh->GetVertexBuffer()->GetBindingDesc() },
		m_pCubeMesh->GetVertexBuffer()->GetAttribDesc(),
		RenderWorkManager::GetDefaultOffscreenRenderPass()
	};

	m_pSkyBoxReflectionMaterial = Material::CreateDefaultMaterial(info);
	m_pSkyBoxReflectionMaterialInstance = m_pSkyBoxReflectionMaterial->CreateMaterialInstance();
	m_pSkyBoxReflectionMaterialInstance->SetRenderMask(1 << GlobalVulkanStates::ReflectionGen);
	m_pSkyBoxReflectionMaterialInstance->GetDescriptorSet(0)->UpdateBufferDynamic(0, m_pUniformBuffer);
	m_pSkyBoxReflectionMaterialInstance->GetDescriptorSet(0)->UpdateImage(1, m_pSkyBoxTex);

	info =
	{
		{ L"../data/shaders/brdf_lut.vert.spv", L"", L"", L"", L"../data/shaders/brdf_lut.frag.spv", L"" },
		{ dsLayoutBindings },
		{ m_pQuadMesh->GetVertexBuffer()->GetBindingDesc() },
		m_pQuadMesh->GetVertexBuffer()->GetAttribDesc(),
		RenderWorkManager::GetDefaultOffscreenRenderPass()
	};

	m_pBRDFLutMaterial = Material::CreateDefaultMaterial(info);
	m_pBRDFLutMaterialInstance = m_pBRDFLutMaterial->CreateMaterialInstance();
	m_pBRDFLutMaterialInstance->SetRenderMask(1 << GlobalVulkanStates::BrdfLutGen);
	m_pBRDFLutMaterialInstance->GetDescriptorSet(0)->UpdateBufferDynamic(0, m_pUniformBuffer);
	m_pBRDFLutMaterialInstance->GetDescriptorSet(0)->UpdateImage(1, m_pSkyBoxTex);
}

void VulkanGlobal::InitEnviromentMap()
{
	RenderWorkMgr()->SetDefaultOffscreenRenderPass(m_pEnvFrameBuffer);

	InitIrradianceMap();
	InitPrefilterEnvMap();
	InitBRDFlutMap();
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
	m_pOffScreenCamObj = BaseObject::Create();
	m_pOffScreenCamComp = Camera::Create(camInfo);
	m_pOffScreenCamObj->AddComponent(m_pOffScreenCamComp);

	m_pCharacter = Character::Create({ 100.0f }, m_pCameraComp);
	m_pCameraObj->AddComponent(m_pCharacter);

	m_pCameraObj->SetPos({ 0, 0, 50 });
	m_pCameraObj->Update();

	m_pGunObject = BaseObject::Create();
	m_pGunMeshRenderer = MeshRenderer::Create(m_pGunMesh, m_pGunMaterialInstance);
	m_pGunObject->AddComponent(m_pGunMeshRenderer);

	m_pSkyBoxObject = BaseObject::Create();
	m_pSkyBoxMeshRenderer = MeshRenderer::Create(m_pCubeMesh, { m_pSkyBoxMaterialInstance, m_pSkyBoxIrradianceMaterialInstance, m_pSkyBoxReflectionMaterialInstance });
	m_pSkyBoxObject->AddComponent(m_pSkyBoxMeshRenderer);

	m_pQuadObject = BaseObject::Create();
	m_pQuadRenderer = MeshRenderer::Create(m_pQuadMesh, m_pBRDFLutMaterialInstance);
	m_pQuadObject->AddComponent(m_pQuadRenderer);

	m_pRootObject = BaseObject::Create();
	m_pRootObject->AddChild(m_pGunObject);
	m_pRootObject->AddChild(m_pSkyBoxObject);
	m_pRootObject->AddChild(m_pQuadObject);
}

void VulkanGlobal::EndSetup()
{
	GlobalDeviceObjects::GetInstance()->GetStagingBufferMgr()->FlushDataMainThread();
}

void VulkanGlobal::UpdateUniforms(uint32_t frameIndex, const std::shared_ptr<Camera>& pCamera)
{
	pCamera->Update(0);
	pCamera->LateUpdate(0);

	memset(&m_globalUniforms, 0, sizeof(GlobalUniforms));

	Matrix4f model;

	Matrix4f vulkanNDC;
	vulkanNDC.c[1].y = -1.0f;
	vulkanNDC.c[2].z = vulkanNDC.c[3].z = 0.5f;

	Matrix4f projMat;
	projMat = pCamera->GetProjMatrix();

	Matrix4f mvp = vulkanNDC * projMat * pCamera->GetViewMatrix() * model;

	Vector3f camPos = pCamera->GetObjectA()->GetWorldPosition();

	memcpy_s(m_globalUniforms.model, sizeof(m_globalUniforms.model), &model, sizeof(model));
	memcpy_s(m_globalUniforms.view, sizeof(m_globalUniforms.view), &pCamera->GetViewMatrix(), sizeof(pCamera->GetViewMatrix()));
	memcpy_s(m_globalUniforms.projection, sizeof(m_globalUniforms.projection), &projMat, sizeof(projMat));
	memcpy_s(m_globalUniforms.vulkanNDC, sizeof(m_globalUniforms.vulkanNDC), &vulkanNDC, sizeof(vulkanNDC));
	memcpy_s(m_globalUniforms.mvp, sizeof(m_globalUniforms.mvp), &mvp, sizeof(mvp));
	memcpy_s(m_globalUniforms.camPos, sizeof(m_globalUniforms.camPos), &camPos, sizeof(camPos));

	if (m_pCameraComp == pCamera)
		m_roughness = 1.0;
	else
		m_roughness = 0.0;
	memcpy_s(&m_globalUniforms.roughness, sizeof(m_globalUniforms.roughness), &m_roughness, sizeof(m_roughness));

	uint32_t totalUniformBytes = m_pUniformBuffer->GetDescBufferInfo().range / GetSwapChain()->GetSwapChainImageCount();
	m_pUniformBuffer->UpdateByteStream(&m_globalUniforms, totalUniformBytes * frameIndex, sizeof(m_globalUniforms));
}

void VulkanGlobal::Draw()
{
	GetSwapChain()->AcquireNextImage();

	GetGlobalVulkanStates()->SetRenderState(GlobalVulkanStates::Scene);
	GetGlobalVulkanStates()->RestoreViewport();
	GetGlobalVulkanStates()->RestoreScissor();

	m_pCharacter->Move(m_moveFlag, 0.001f);

	RenderWorkMgr()->SetDefaultRenderPass(GlobalObjects()->GetCurrentFrameBuffer());

	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = m_perFrameRes[FrameMgr()->FrameIndex()]->AllocatePrimaryCommandBuffer();
	pDrawCmdBuffer->StartPrimaryRecording();

	VulkanGlobal::GetInstance()->UpdateUniforms(FrameMgr()->FrameIndex(), VulkanGlobal::GetInstance()->m_pCameraComp);
	StagingBufferMgr()->RecordDataFlush(pDrawCmdBuffer);

	std::vector<VkClearValue> clearValues =
	{
		{ 0.2f, 0.2f, 0.2f, 0.2f },
		{ 1.0f, 0 }
	};
	pDrawCmdBuffer->BeginRenderPass(clearValues, true);

	m_pRootObject->Update();

	GlobalObjects()->GetCurrentFrameBuffer()->GetRenderPass()->ExecuteCachedSecondaryCommandBuffers(pDrawCmdBuffer);

	pDrawCmdBuffer->EndRenderPass();

	pDrawCmdBuffer->EndPrimaryRecording();
	FrameMgr()->CacheSubmissioninfo(GlobalGraphicQueue(), { pDrawCmdBuffer }, {}, false);

	GetSwapChain()->QueuePresentImage(GlobalObjects()->GetPresentQueue());
}

void VulkanGlobal::Init(HINSTANCE hInstance, WNDPROC wndproc)
{
	SetupWindow(hInstance, wndproc);
	InitVulkanInstance();
	InitPhysicalDevice(m_hPlatformInst, m_hWindow);
	InitSurface();
	InitVulkanDevice();
	GlobalDeviceObjects::GetInstance()->Init(m_pDevice);
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
	InitEnviromentMap();
	EndSetup();
}