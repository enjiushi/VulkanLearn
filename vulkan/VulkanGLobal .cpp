#include "VulkanGlobal.h"
#include "../common/Macros.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include <array>
#include "../thread/ThreadCoordinator.h"
#include "../maths/Matrix.h"
#include <math.h>
#include "Importer.hpp"
#include "scene.h"
#include "postprocess.h"
#include "Buffer.h"
#include "StagingBuffer.h"

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

	m_vulkanInst = Instance::Create(instCreateInfo);
	assert(m_vulkanInst != nullptr);
}

void VulkanGlobal::InitPhysicalDevice(HINSTANCE hInstance, HWND hWnd)
{
	m_physicalDevice = PhysicalDevice::Create(m_vulkanInst, hInstance, hWnd);
	ASSERTION(m_physicalDevice != nullptr);
}

void VulkanGlobal::InitVulkanDevice()
{
	m_pDevice = Device::Create(m_vulkanInst, m_physicalDevice);
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

void VulkanGlobal::HandleMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		ThreadCoordinator::Free();
		break;
	}
}

#endif

void VulkanGlobal::InitQueue()
{
	vkGetDeviceQueue(m_pDevice->GetDeviceHandle(), m_physicalDevice->GetGraphicQueueIndex(), 0, &m_queue);
}

void VulkanGlobal::InitSurface()
{
}

void VulkanGlobal::InitSwapchain()
{
	m_pSwapchain = SwapChain::Create(m_pDevice);
	ASSERTION(m_pSwapchain != nullptr);
}

void VulkanGlobal::Update()
{
#if defined(_WIN32)
	static uint32_t frameCount = 0;
	static float fpsTimer = 0;
	MSG msg;
	while (TRUE)
	{
		auto tStart = std::chrono::high_resolution_clock::now();
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
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
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = m_physicalDevice->GetGraphicQueueIndex();
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	CHECK_VK_ERROR(vkCreateCommandPool(m_pDevice->GetDeviceHandle(), &commandPoolCreateInfo, nullptr, &m_commandPool));
}

void VulkanGlobal::InitSetupCommandBuffer()
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = m_commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	CHECK_VK_ERROR(vkAllocateCommandBuffers(m_pDevice->GetDeviceHandle(), &commandBufferAllocateInfo, &m_setupCommandBuffer));
	CHECK_VK_ERROR(vkAllocateCommandBuffers(m_pDevice->GetDeviceHandle(), &commandBufferAllocateInfo, &m_prePresentCmdBuffer));
	CHECK_VK_ERROR(vkAllocateCommandBuffers(m_pDevice->GetDeviceHandle(), &commandBufferAllocateInfo, &m_postPresentCmdBuffer));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CHECK_VK_ERROR(vkBeginCommandBuffer(m_setupCommandBuffer, &beginInfo));
}

void VulkanGlobal::InitMemoryMgr()
{
	m_pMemoryMgr = DeviceMemoryManager::Create(m_pDevice);
	assert(m_pMemoryMgr != nullptr);
}

void VulkanGlobal::InitSwapchainImgs()
{
	m_swapchainImg.images.resize(m_physicalDevice->GetSurfaceCap().maxImageCount);
	m_swapchainImg.views.resize(m_physicalDevice->GetSurfaceCap().maxImageCount);
	uint32_t count = 0;
	CHECK_VK_ERROR(vkGetSwapchainImagesKHR(m_pDevice->GetDeviceHandle(), m_pSwapchain->GetDeviceHandle(), &count, nullptr));
	ASSERTION(count == m_physicalDevice->GetSurfaceCap().maxImageCount);
	CHECK_VK_ERROR(vkGetSwapchainImagesKHR(m_pDevice->GetDeviceHandle(), m_pSwapchain->GetDeviceHandle(), &count, m_swapchainImg.images.data()));

	for (uint32_t i = 0; i < m_physicalDevice->GetSurfaceCap().maxImageCount; i++)
	{
		//Create image view
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.format = m_physicalDevice->GetSurfaceFormat().format;
		imageViewCreateInfo.image = m_swapchainImg.images[i];
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.components =
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		CHECK_VK_ERROR(vkCreateImageView(m_pDevice->GetDeviceHandle(), &imageViewCreateInfo, nullptr, &m_swapchainImg.views[i]));

		//Change image layout
		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.image = m_swapchainImg.images[i];
		imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.subresourceRange.baseMipLevel = 0;
		imageBarrier.subresourceRange.levelCount = 1;
		imageBarrier.subresourceRange.baseArrayLayer = 0;
		imageBarrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(m_setupCommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageBarrier);
	}
}

void VulkanGlobal::InitDepthStencil()
{
	//create image of depth stencil
	VkImageCreateInfo dsCreateInfo = {};
	dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	dsCreateInfo.format = m_physicalDevice->GetDepthStencilFormat();
	dsCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	dsCreateInfo.arrayLayers = 1;
	dsCreateInfo.extent.depth = 1;
	dsCreateInfo.extent.width = m_physicalDevice->GetSurfaceCap().currentExtent.width;
	dsCreateInfo.extent.height = m_physicalDevice->GetSurfaceCap().currentExtent.height;
	dsCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	dsCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	dsCreateInfo.mipLevels = 1;
	dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	CHECK_VK_ERROR(vkCreateImage(m_pDevice->GetDeviceHandle(), &dsCreateInfo, nullptr, &m_depthStencil.image));

	//Get memory requirements from depth stencil image
	VkMemoryRequirements dsReq = {};
	vkGetImageMemoryRequirements(m_pDevice->GetDeviceHandle(), m_depthStencil.image, &dsReq);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = dsReq.size;

	uint32_t typeBits = dsReq.memoryTypeBits;
	uint32_t typeIndex = 0;
	while (typeBits)
	{
		if (typeBits & 1)
		{
			if (m_physicalDevice->GetPhysicalDeviceMemoryProperties().memoryTypes[typeIndex].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				memoryAllocateInfo.memoryTypeIndex = typeIndex;
				break;
			}
		}
		typeBits >>= 1;
		typeIndex++;
	}

	CHECK_VK_ERROR(vkAllocateMemory(m_pDevice->GetDeviceHandle(), &memoryAllocateInfo, nullptr, &m_depthStencil.memory));

	//Bind memory to depth stencil image
	CHECK_VK_ERROR(vkBindImageMemory(m_pDevice->GetDeviceHandle(), m_depthStencil.image, m_depthStencil.memory, 0));

	//Create depth stencil image view
	VkImageViewCreateInfo dsImageViewCreateInfo = {};
	dsImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	dsImageViewCreateInfo.image = m_depthStencil.image;
	dsImageViewCreateInfo.format = m_physicalDevice->GetDepthStencilFormat();
	dsImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	dsImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	dsImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	dsImageViewCreateInfo.subresourceRange.layerCount = 1;
	dsImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	dsImageViewCreateInfo.subresourceRange.levelCount = 1;

	CHECK_VK_ERROR(vkCreateImageView(m_pDevice->GetDeviceHandle(), &dsImageViewCreateInfo, nullptr, &m_depthStencil.view));

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = m_depthStencil.image;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barrier.dstQueueFamilyIndex = m_physicalDevice->GetGraphicQueueIndex();
	barrier.srcAccessMask = 0;
	barrier.srcQueueFamilyIndex = m_physicalDevice->GetGraphicQueueIndex();
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;

	vkCmdPipelineBarrier(m_setupCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
}

void VulkanGlobal::InitRenderpass()
{
	std::vector<VkAttachmentDescription> attachmentDescs(2);
	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescs[0].format = m_physicalDevice->GetSurfaceFormat().format;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[1].format = m_physicalDevice->GetDepthStencilFormat();
	attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference colorAttach = {};
	colorAttach.attachment = 0;
	colorAttach.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference dsAttach = {};
	dsAttach.attachment = 1;
	dsAttach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttach;
	subpass.pDepthStencilAttachment = &dsAttach;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = attachmentDescs.size();
	renderpassCreateInfo.pAttachments = attachmentDescs.data();
	renderpassCreateInfo.subpassCount = 1;
	renderpassCreateInfo.pSubpasses = &subpass;

	CHECK_VK_ERROR(vkCreateRenderPass(m_pDevice->GetDeviceHandle(), &renderpassCreateInfo, nullptr, &m_renderpass));
}

void VulkanGlobal::InitFrameBuffer()
{
	m_framebuffers.resize(m_physicalDevice->GetSurfaceCap().maxImageCount);
	std::vector<VkImageView> attachments(2);
	attachments[1] = m_depthStencil.view;

	for (uint32_t i = 0; i < m_physicalDevice->GetSurfaceCap().maxImageCount; i++)
	{
		attachments[0] = m_swapchainImg.views[i];

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.attachmentCount = 2;
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.width = m_physicalDevice->GetSurfaceCap().currentExtent.width;
		framebufferCreateInfo.height = m_physicalDevice->GetSurfaceCap().currentExtent.height;
		framebufferCreateInfo.renderPass = m_renderpass;

		CHECK_VK_ERROR(vkCreateFramebuffer(m_pDevice->GetDeviceHandle(), &framebufferCreateInfo, nullptr, &m_framebuffers[i]));
	}
}

void VulkanGlobal::InitVertices()
{
	Assimp::Importer imp;
	const aiScene* pScene = nullptr;
	pScene = imp.ReadFile("data/models/sphere.obj", aiProcess_Triangulate | aiProcess_GenSmoothNormals);

	aiMesh* pMesh = pScene->mMeshes[0];

	uint32_t vertexSize = 0;
	if (pMesh->HasPositions())
		vertexSize += 3 * sizeof(float);
	if (pMesh->HasNormals())
		vertexSize += 3 * sizeof(float);
	//FIXME: hard-coded index 0 here, we don't have more than 1 color for now
	if (pMesh->HasVertexColors(0))
		vertexSize += 4 * sizeof(float);
	//FIXME: hard-coded index 0 here, we don't have more than 1 texture coord for now
	if (pMesh->HasTextureCoords(0))
		vertexSize += 3 * sizeof(float);
	if (pMesh->HasTangentsAndBitangents())
		vertexSize += 6 * sizeof(float);

	float* pVertices = new float[pMesh->mNumVertices * 9];
	uint32_t verticesNumBytes = pMesh->mNumVertices * vertexSize;
	uint32_t count = 0;

	for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
	{
		uint32_t offset = i * vertexSize / sizeof(float);
		count = 0;
		if (pMesh->HasPositions())
		{
			pVertices[offset] = pMesh->mVertices[i].x;
			pVertices[offset + 1] = pMesh->mVertices[i].y;
			pVertices[offset + 2] = pMesh->mVertices[i].z;
			count += 3;
		}
		if (pMesh->HasNormals())
		{
			pVertices[offset + count] = pMesh->mNormals[i].x;
			pVertices[offset + count + 1] = pMesh->mNormals[i].y;
			pVertices[offset + count + 2] = pMesh->mNormals[i].z;
			count += 3;
		}
		if (pMesh->HasVertexColors(0))
		{
			pVertices[offset + count] = pMesh->mColors[i][0].r;
			pVertices[offset + count + 1] = pMesh->mColors[i][0].g;
			pVertices[offset + count + 2] = pMesh->mColors[i][0].b;
			pVertices[offset + count + 3] = pMesh->mColors[i][0].a;
			count += 4;
		}
		if (pMesh->HasTextureCoords(0))
		{
			pVertices[offset + count] = pMesh->mTextureCoords[0][i].x;
			pVertices[offset + count + 1] = pMesh->mTextureCoords[0][i].y;
			pVertices[offset + count + 2] = pMesh->mTextureCoords[0][i].z;
			count += 3;
		}
		if (pMesh->HasTangentsAndBitangents())
		{
			pVertices[offset + count] = pMesh->mTangents[i].x;
			pVertices[offset + count + 1] = pMesh->mTangents[i].y;
			pVertices[offset + count + 2] = pMesh->mTangents[i].z;
			pVertices[offset + count + 3] = pMesh->mBitangents[i].x;
			pVertices[offset + count + 4] = pMesh->mBitangents[i].y;
			pVertices[offset + count + 5] = pMesh->mBitangents[i].z;
			count += 6;
		}
	}

	uint32_t* pIndices = new uint32_t[pMesh->mNumFaces * 3];
	uint32_t indicesNumBytes = pMesh->mNumFaces * 3 * sizeof(uint32_t);
	for (size_t i = 0; i < pMesh->mNumFaces; i++)
	{
		pIndices[i * 3] = pMesh->mFaces[i].mIndices[0];
		pIndices[i * 3 + 1] = pMesh->mFaces[i].mIndices[1];
		pIndices[i * 3 + 2] = pMesh->mFaces[i].mIndices[2];
	}

	std::shared_ptr<StagingBuffer> stageVertexBuffer, stageIndexBuffer;
	stageVertexBuffer = StagingBuffer::Create(m_pDevice, verticesNumBytes, m_pMemoryMgr, pVertices);
	stageIndexBuffer = StagingBuffer::Create(m_pDevice, indicesNumBytes, m_pMemoryMgr, pIndices);

	//Binding and attributes information
	VkVertexInputBindingDescription bindingDesc = {};
	bindingDesc.binding = 0;		//hard coded 0
	bindingDesc.stride = vertexSize;	//xyzrgb, all hard coded, mockup code, don't care, just for learning
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> attribDesc;
	attribDesc.resize(3);

	attribDesc[0].binding = 0;
	attribDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc[0].location = 0;	//layout location 0 in shader
	attribDesc[0].offset = 0;

	attribDesc[1].binding = 0;
	attribDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc[1].location = 1;
	attribDesc[1].offset = sizeof(float) * 3;	//after xyz*/

	attribDesc[2].binding = 0;
	attribDesc[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc[2].location = 2;
	attribDesc[2].offset = sizeof(float) * 6;	//after xyz*/

	m_vertexBuffer = VertexBuffer::Create(m_pDevice, verticesNumBytes, bindingDesc, attribDesc, m_pMemoryMgr);

	m_indexBuffer = IndexBuffer::Create(m_pDevice, indicesNumBytes, VK_INDEX_TYPE_UINT32, m_pMemoryMgr);

	//Setup a barrier for staging buffers
	VkBufferMemoryBarrier barriers[2] = {};

	barriers[0] = {};
	barriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barriers[0].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barriers[0].buffer = stageVertexBuffer->GetDeviceHandle();
	barriers[0].offset = 0;
	barriers[0].size = stageVertexBuffer->GetBufferInfo().size;

	barriers[1] = {};
	barriers[1].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barriers[1].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barriers[1].buffer = stageIndexBuffer->GetDeviceHandle();
	barriers[1].offset = 0;
	barriers[1].size = stageIndexBuffer->GetBufferInfo().size;

	vkCmdPipelineBarrier(m_setupCommandBuffer,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		2, &barriers[0],
		0, nullptr);

	//Transfer data from staging buffers to device local buffers
	//Copy vertex buffer
	VkBufferCopy copy = {};
	copy.dstOffset = 0;
	copy.srcOffset = 0;
	copy.size = stageVertexBuffer->GetBufferInfo().size;
	vkCmdCopyBuffer(m_setupCommandBuffer, stageVertexBuffer->GetDeviceHandle(), m_vertexBuffer->GetDeviceHandle(), 1, &copy);

	//Copy index buffer
	copy.size = stageIndexBuffer->GetBufferInfo().size;
	vkCmdCopyBuffer(m_setupCommandBuffer, stageIndexBuffer->GetDeviceHandle(), m_indexBuffer->GetDeviceHandle(), 1, &copy);

	//Setup a barrier for memory changes
	barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barriers[0].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	barriers[0].buffer = m_vertexBuffer->GetDeviceHandle();

	barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barriers[1].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;	//not sure if it's the same as vertex buffer
	barriers[1].buffer = m_indexBuffer->GetDeviceHandle();

	vkCmdPipelineBarrier(m_setupCommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		0,
		0, nullptr,
		2, &barriers[0],
		0, nullptr);
}

void VulkanGlobal::InitUniforms()
{
	uint32_t totalUniformBytes = sizeof(m_mvp.model) * 5 + sizeof(m_mvp.camPos);

	memset(m_mvp.model, 0, sizeof(m_mvp.model));
	memset(m_mvp.view, 0, sizeof(m_mvp.view));
	memset(m_mvp.projection, 0, sizeof(m_mvp.projection));
	memset(m_mvp.vulkanNDC, 0, sizeof(m_mvp.vulkanNDC));
	memset(m_mvp.mvp, 0, sizeof(m_mvp.mvp));

	Matrix4f model;

	Matrix4f view;
	Vector3f up = { 0, 1, 0 };
	//Vector3f look = { 1, -1, -1 };
	Vector3f look = { 0, 0, -1 };
	look.Normalize();
	Vector3f position = { 0, 0, 50 };
	Vector3f xaxis = up ^ look.Negativate();
	xaxis.Normalize();
	Vector3f yaxis = look ^ xaxis;
	yaxis.Normalize();

	view.c[0] = xaxis;
	view.c[1] = yaxis;
	view.c[2] = look;
	view.c[3] = position;
	view.Inverse();

	Matrix4f projection;
	float aspect = 1024.0 / 768.0;
	float fov = 3.1415 / 2.0;
	float nearPlane = 1;
	float farPlane = 200;

	projection.c[0] = { 1.0f / (nearPlane * std::tanf(fov / 2.0f) * aspect), 0, 0, 0 };
	projection.c[1] = { 0, 1.0f / (nearPlane * std::tanf(fov / 2.0f)), 0, 0 };
	projection.c[2] = { 0, 0, (nearPlane + farPlane) / (nearPlane - farPlane), -1 };
	projection.c[3] = { 0, 0, 2.0f * nearPlane * farPlane / (nearPlane - farPlane), 0 };

	Matrix4f vulkanNDC;
	vulkanNDC.c[1].y = -1.0f;
	vulkanNDC.c[2].z = vulkanNDC.c[3].z = 0.5f;

	Matrix4f mvp = vulkanNDC * projection * view * model;

	Vector3f camPos = position;

	memcpy_s(m_mvp.model, sizeof(m_mvp.model), &model, sizeof(model));
	memcpy_s(m_mvp.view, sizeof(m_mvp.view), &view, sizeof(view));
	memcpy_s(m_mvp.projection, sizeof(m_mvp.projection), &projection, sizeof(projection));
	memcpy_s(m_mvp.vulkanNDC, sizeof(m_mvp.vulkanNDC), &vulkanNDC, sizeof(vulkanNDC));
	memcpy_s(m_mvp.mvp, sizeof(m_mvp.mvp), &mvp, sizeof(mvp));
	memcpy_s(m_mvp.camPos, sizeof(m_mvp.camPos), &camPos, sizeof(camPos));

	std::shared_ptr<StagingBuffer> pStagingBuffer = StagingBuffer::Create(m_pDevice, totalUniformBytes, m_pMemoryMgr, &m_mvp);
	m_uniformBuffer = UniformBuffer::Create(m_pDevice, totalUniformBytes, m_pMemoryMgr);

	//m_uniformBuffer.buffer.Init(m_pDevice, m_uniformBuffer.info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_pMemoryMgr, &m_mvp);

	//Setup a barrier to let shader know its latest updated information in buffer
	VkBufferMemoryBarrier barrier = {};
	barrier.buffer = pStagingBuffer->GetDeviceHandle();
	barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.offset = 0;
	barrier.size = totalUniformBytes;
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

	vkCmdPipelineBarrier(m_setupCommandBuffer,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		1, &barrier,
		0, nullptr);

	VkBufferCopy copy = {};
	copy.dstOffset = 0;
	copy.srcOffset = 0;
	copy.size = m_uniformBuffer->GetBufferInfo().size;
	vkCmdCopyBuffer(m_setupCommandBuffer, pStagingBuffer->GetDeviceHandle(), m_uniformBuffer->GetDeviceHandle(), 1, &copy);

	barrier.buffer = m_uniformBuffer->GetDeviceHandle();
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(m_setupCommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		0,
		0, nullptr,
		1, &barrier,
		0, nullptr);
}

void VulkanGlobal::InitDescriptorSetLayout()
{
	m_dsLayoutBinding =
	{
		{
			0,	//binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	//type
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		},
	};

	VkDescriptorSetLayoutCreateInfo dslayoutInfo = {};
	dslayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dslayoutInfo.bindingCount = m_dsLayoutBinding.size();
	dslayoutInfo.pBindings = m_dsLayoutBinding.data();
	CHECK_VK_ERROR(vkCreateDescriptorSetLayout(m_pDevice->GetDeviceHandle(), &dslayoutInfo, nullptr, &m_descriptorSetLayout));

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	CHECK_VK_ERROR(vkCreatePipelineLayout(m_pDevice->GetDeviceHandle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
}

void VulkanGlobal::InitPipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	CHECK_VK_ERROR(vkCreatePipelineCache(m_pDevice->GetDeviceHandle(), &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));
}

void VulkanGlobal::InitPipeline()
{
	VkPipelineColorBlendAttachmentState blendState = {};
	blendState.colorWriteMask = 0xf;
	blendState.blendEnable = VK_FALSE;

	blendState.colorBlendOp = VK_BLEND_OP_ADD;
	blendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

	blendState.alphaBlendOp = VK_BLEND_OP_ADD;
	blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

	VkPipelineColorBlendStateCreateInfo blendStateInfo = {};
	blendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendStateInfo.logicOpEnable = VK_FALSE;
	blendStateInfo.attachmentCount = 1;
	blendStateInfo.pAttachments = &blendState;

	VkPipelineDepthStencilStateCreateInfo dsCreateInfo = {};
	dsCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	dsCreateInfo.depthTestEnable = VK_TRUE;
	dsCreateInfo.depthWriteEnable = VK_TRUE;
	dsCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	VkPipelineInputAssemblyStateCreateInfo assCreateInfo = {};
	assCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineMultisampleStateCreateInfo msCreateInfo = {};
	msCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineRasterizationStateCreateInfo rsCreateInfo = {};
	rsCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rsCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rsCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rsCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rsCreateInfo.lineWidth = 1.0f;
	rsCreateInfo.depthClampEnable = VK_FALSE;
	rsCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rsCreateInfo.depthBiasEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo vpCreateInfo = {};
	vpCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vpCreateInfo.viewportCount = 1;
	vpCreateInfo.pScissors = nullptr;
	vpCreateInfo.scissorCount = 1;
	vpCreateInfo.pViewports = nullptr;

	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pColorBlendState = &blendStateInfo;
	pipelineInfo.pDepthStencilState = &dsCreateInfo;
	pipelineInfo.pInputAssemblyState = &assCreateInfo;
	pipelineInfo.pMultisampleState = &msCreateInfo;
	pipelineInfo.pRasterizationState = &rsCreateInfo;
	pipelineInfo.pViewportState = &vpCreateInfo;
	pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineInfo.renderPass = m_renderpass;
	pipelineInfo.layout = m_pipelineLayout;

	VkShaderModule vertex_module = InitShaderModule("data/shaders/simple.vert.spv");
	VkShaderModule fragment_module = InitShaderModule("data/shaders/simple.frag.spv");

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	shaderStages[0] = shaderStages[1] = {};
	shaderStages[0].sType = shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].pName = "main";
	shaderStages[0].module = vertex_module;

	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].pName = "main";
	shaderStages[1].module = fragment_module;

	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();

	VkPipelineVertexInputStateCreateInfo vertexCreateInfo = {};
	vertexCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexCreateInfo.vertexBindingDescriptionCount = 1;
	vertexCreateInfo.pVertexBindingDescriptions = &m_vertexBuffer->GetBindingDesc();
	vertexCreateInfo.vertexAttributeDescriptionCount = m_vertexBuffer->GetAttribDesc().size();
	vertexCreateInfo.pVertexAttributeDescriptions = m_vertexBuffer->GetAttribDesc().data();
	pipelineInfo.pVertexInputState = &vertexCreateInfo;

	CHECK_VK_ERROR(vkCreateGraphicsPipelines(m_pDevice->GetDeviceHandle(), 0, 1, &pipelineInfo, nullptr, &m_pipeline));
}

VkShaderModule VulkanGlobal::InitShaderModule(const char* shaderPath)
{
	std::ifstream ifs;
	ifs.open(shaderPath, std::ios::binary);
	assert(ifs.good());
	std::vector<char> buffer;
	buffer.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

	VkShaderModule module;
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = buffer.size();
	shaderModuleCreateInfo.pCode = (uint32_t*)buffer.data();
	CHECK_VK_ERROR(vkCreateShaderModule(m_pDevice->GetDeviceHandle(), &shaderModuleCreateInfo, nullptr, &module));
	return module;
}

void VulkanGlobal::InitDescriptorPool()
{
	VkDescriptorPoolSize descPoolSize = {};
	descPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize.descriptorCount = 3;

	VkDescriptorPoolCreateInfo descPoolInfo = {};
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pPoolSizes = &descPoolSize;
	descPoolInfo.poolSizeCount = 1;
	descPoolInfo.maxSets = 12;

	CHECK_VK_ERROR(vkCreateDescriptorPool(m_pDevice->GetDeviceHandle(), &descPoolInfo, nullptr, &m_descriptorPool));
}

void VulkanGlobal::InitDescriptorSet()
{
	VkDescriptorSetAllocateInfo descAllocInfo = {};
	descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descAllocInfo.descriptorPool = m_descriptorPool;
	descAllocInfo.descriptorSetCount = 1;
	descAllocInfo.pSetLayouts = &m_descriptorSetLayout;

	CHECK_VK_ERROR(vkAllocateDescriptorSets(m_pDevice->GetDeviceHandle(), &descAllocInfo, &m_descriptorSet));

	std::vector<VkWriteDescriptorSet> writeDescSet;
	writeDescSet.resize(1);
	
	VkDescriptorBufferInfo info = m_uniformBuffer->GetDescBufferInfo();
	writeDescSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescSet[0].dstBinding = 0;
	writeDescSet[0].dstSet = m_descriptorSet;
	writeDescSet[0].pBufferInfo = &info;
	writeDescSet[0].descriptorCount = 1;

	vkUpdateDescriptorSets(m_pDevice->GetDeviceHandle(), writeDescSet.size(), writeDescSet.data(), 0, nullptr);
}

void VulkanGlobal::InitDrawCmdBuffers()
{
	m_drawCmdBuffers.resize(m_swapchainImg.images.size());

	VkCommandBufferAllocateInfo cmdAllocInfo = {};
	cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocInfo.commandPool = m_commandPool;
	cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdAllocInfo.commandBufferCount = m_swapchainImg.images.size();
	CHECK_VK_ERROR(vkAllocateCommandBuffers(m_pDevice->GetDeviceHandle(), &cmdAllocInfo, m_drawCmdBuffers.data()));

	for (size_t i = 0; i < m_swapchainImg.images.size(); i++)
	{
		VkCommandBufferBeginInfo cmdBeginInfo = {};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CHECK_VK_ERROR(vkBeginCommandBuffer(m_drawCmdBuffers[i], &cmdBeginInfo));

		std::vector<VkClearValue> clearValues =
		{
			{ 0.2f, 0.2f, 0.2f, 0.2f },
			{ 1.0f, 0 }
		};

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.renderPass = m_renderpass;
		renderPassBeginInfo.framebuffer = m_framebuffers[i];
		renderPassBeginInfo.renderArea.extent.width = m_physicalDevice->GetSurfaceCap().currentExtent.width;
		renderPassBeginInfo.renderArea.extent.height = m_physicalDevice->GetSurfaceCap().currentExtent.height;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;

		vkCmdBeginRenderPass(m_drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport =
		{
			0, 0,
			m_physicalDevice->GetSurfaceCap().currentExtent.width, m_physicalDevice->GetSurfaceCap().currentExtent.height,
			0, 1
		};

		VkRect2D scissorRect =
		{
			0, 0,
			m_physicalDevice->GetSurfaceCap().currentExtent.width, m_physicalDevice->GetSurfaceCap().currentExtent.height
		};

		vkCmdSetViewport(m_drawCmdBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(m_drawCmdBuffers[i], 0, 1, &scissorRect);

		vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);

		vkCmdBindPipeline(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		VkDeviceSize deviceSize[1] = { 0 };
		VkBuffer vertexBuffer = m_vertexBuffer->GetDeviceHandle();
		vkCmdBindVertexBuffers(m_drawCmdBuffers[i], 0, 1, &vertexBuffer, deviceSize);
		vkCmdBindIndexBuffer(m_drawCmdBuffers[i], m_indexBuffer->GetDeviceHandle(), 0, m_indexBuffer->GetType());

		vkCmdDrawIndexed(m_drawCmdBuffers[i], m_indexBuffer->GetCount(), 1, 0, 0, 0);

		vkCmdEndRenderPass(m_drawCmdBuffers[i]);

		CHECK_VK_ERROR(vkEndCommandBuffer(m_drawCmdBuffers[i]));
	}
}

void VulkanGlobal::InitSemaphore()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	CHECK_VK_ERROR(vkCreateSemaphore(m_pDevice->GetDeviceHandle(), &semaphoreInfo, nullptr, &m_swapchainAcquireDone));
	CHECK_VK_ERROR(vkCreateSemaphore(m_pDevice->GetDeviceHandle(), &semaphoreInfo, nullptr, &m_renderDone));
}

void VulkanGlobal::EndSetup()
{
	if (m_setupCommandBuffer == nullptr)
		return;

	CHECK_VK_ERROR(vkEndCommandBuffer(m_setupCommandBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_setupCommandBuffer;

	CHECK_VK_ERROR(vkQueueSubmit(m_queue, 1, &submitInfo, nullptr));
	vkQueueWaitIdle(m_queue);

	vkFreeCommandBuffers(m_pDevice->GetDeviceHandle(), m_commandPool, 1, &m_setupCommandBuffer);
}

void VulkanGlobal::Draw()
{
	CHECK_VK_ERROR(m_pSwapchain->GetAcquireNextImageFuncPtr()(m_pDevice->GetDeviceHandle(), m_pSwapchain->GetDeviceHandle(), UINT64_MAX, m_swapchainAcquireDone, nullptr, &m_currentBufferIndex));

	VkPipelineStageFlags flag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_drawCmdBuffers[m_currentBufferIndex];
	submitInfo.pWaitDstStageMask = &flag;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_swapchainAcquireDone;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_renderDone;

	vkQueueSubmit(m_queue, 1, &submitInfo, nullptr);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	VkSwapchainKHR swapchain = (m_pSwapchain->GetDeviceHandle());
	presentInfo.pSwapchains = &swapchain;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderDone;
	presentInfo.pImageIndices = &m_currentBufferIndex;
	m_pSwapchain->GetQueuePresentFuncPtr()(m_queue, &presentInfo);

	vkQueueWaitIdle(m_queue);
}

void VulkanGlobal::Init(HINSTANCE hInstance, WNDPROC wndproc)
{
	SetupWindow(hInstance, wndproc);
	InitVulkanInstance();
	InitPhysicalDevice(m_hPlatformInst, m_hWindow);
	InitSurface();
	InitVulkanDevice();
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
	InitPipelineCache();
	InitPipeline();
	InitDescriptorPool();
	InitDescriptorSet();
	InitDrawCmdBuffers();
	InitSemaphore();
	EndSetup();
}