#include "VulkanLearn.h"
#include "Macros.h"
#include <iostream>
#include <chrono>
#include <sstream>

VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
	int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {

	OutputDebugStringA(pLayerPrefix);
	OutputDebugStringA(" ");
	OutputDebugStringA(pMessage);
	OutputDebugStringA("\n");
	return VK_FALSE;
}

void VulkanInstance::InitVulkanInstance()
{
	VkApplicationInfo appInfo	= {};
	appInfo.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName	= "VulkanLearn";
	appInfo.apiVersion			= (((1) << 22) | ((0) << 12) | (0));

	//Need surface extension to create surface from device
	std::vector<const char*> extensions = { EXTENSION_VULKAN_SURFACE };
	std::vector<const char*> layers;
#if defined(_WIN32)
	extensions.push_back( EXTENSION_VULKAN_SURFACE_WIN32 );
#endif
#if defined(_DEBUG)
	layers.push_back(EXTENSION_VULKAN_VALIDATION_LAYER);
	extensions.push_back(EXTENSION_VULKAN_DEBUG_REPORT);
#endif
	VkInstanceCreateInfo instCreateInfo = {};
	instCreateInfo.sType				= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instCreateInfo.pApplicationInfo		= &appInfo;
	instCreateInfo.enabledExtensionCount = (int32_t)extensions.size();
	instCreateInfo.ppEnabledExtensionNames = extensions.data();
	instCreateInfo.enabledLayerCount = (int32_t)layers.size();
	instCreateInfo.ppEnabledLayerNames = layers.data();

	CHECK_VK_ERROR(vkCreateInstance(&instCreateInfo, nullptr, &m_vulkanInst));

	GET_INSTANCE_PROC_ADDR(m_vulkanInst, CreateDebugReportCallbackEXT);
	GET_INSTANCE_PROC_ADDR(m_vulkanInst, DebugReportMessageEXT);
	GET_INSTANCE_PROC_ADDR(m_vulkanInst, DestroyDebugReportCallbackEXT);

	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = &MyDebugReportCallback;
	callbackCreateInfo.pUserData = NULL;

	VkResult res = m_fpCreateDebugReportCallbackEXT(m_vulkanInst, &callbackCreateInfo, NULL, &m_debugCallback);
	ASSERTION(res == VK_SUCCESS);

	GET_INSTANCE_PROC_ADDR(m_vulkanInst, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(m_vulkanInst, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(m_vulkanInst, GetPhysicalDeviceSurfacePresentModesKHR);
	GET_INSTANCE_PROC_ADDR(m_vulkanInst, GetPhysicalDeviceSurfaceSupportKHR);
}

void VulkanInstance::InitPhysicalDevice()
{
	//Get an available physical device
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices(m_vulkanInst, &gpuCount, nullptr);
	ASSERTION(gpuCount > 0);
	std::vector<VkPhysicalDevice> physicalDevices;
	physicalDevices.resize(gpuCount);
	vkEnumeratePhysicalDevices(m_vulkanInst, &gpuCount, physicalDevices.data());
	m_physicalDevice = physicalDevices[0];

	//Get queue properties from physical device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
	ASSERTION(queueFamilyCount > 0);
	m_queueProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueProperties.data());

	//Get physical device properties
	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_physicalDeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties);

	//Get depth stencil format
	std::vector<VkFormat> formats =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (uint32_t i = 0; i < formats.size(); i++)
	{
		VkFormatProperties prop;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, formats[i], &prop);
		if (prop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			m_depthStencil.format = formats[i];
			break;
		}
	}
}

void VulkanInstance::InitVulkanDevice()
{
	uint32_t queueIndex = 0;
	for (; queueIndex < m_queueProperties.size(); queueIndex++)
	{
		if (m_queueProperties[queueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			break;
	}
	ASSERTION(queueIndex < m_queueProperties.size());
	m_graphicQueueIndex = queueIndex;

	std::array<float, 1> queueProperties = { 0.0f };
	VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex = m_graphicQueueIndex;
	deviceQueueCreateInfo.queueCount = queueProperties.size();
	deviceQueueCreateInfo.pQueuePriorities = queueProperties.data();

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	std::vector<const char*> extensions = { EXTENSION_VULKAN_SWAPCHAIN };
	deviceCreateInfo.enabledExtensionCount = extensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

	CHECK_VK_ERROR(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device));

	GET_DEVICE_PROC_ADDR(m_device, CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(m_device, DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(m_device, GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(m_device, AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(m_device, QueuePresentKHR);
}

#if defined (_WIN32)
void VulkanInstance::SetupWindow(HINSTANCE hinstance, WNDPROC wndproc)
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

void VulkanInstance::HandleMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	}
}

#endif

void VulkanInstance::InitSurface()
{
#if defined (_WIN32)
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)m_hPlatformInst;
	surfaceCreateInfo.hwnd = (HWND)m_hWindow;
	CHECK_VK_ERROR(vkCreateWin32SurfaceKHR(m_vulkanInst, &surfaceCreateInfo, nullptr, &m_surface));
#endif

	//Get all queues information whether they support presentation or not
	std::vector<VkBool32> supportPresent(m_queueProperties.size());
	for (uint32_t i = 0; i < m_queueProperties.size(); i++)
	{
		CHECK_VK_ERROR(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_graphicQueueIndex, m_surface, &supportPresent[i]));
	}

	//Store the first one supports presentation
	for (uint32_t i = 0; i < supportPresent.size(); i++)
	{
		if (supportPresent[i])
		{
			m_presentQueueIndex = i;
			break;
		}
	}

	//Get surface's format
	uint32_t formatCount = 0;
	CHECK_VK_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr));
	ASSERTION(formatCount > 0);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	CHECK_VK_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data()));
	m_surfaceFormat = formats[0];
}

void VulkanInstance::InitSwapchain()
{
	//Get surface capabilities
	VkSurfaceCapabilitiesKHR surfCap = {};
	CHECK_VK_ERROR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfCap));

	//Get present modes
	uint32_t presentModeCount = 0;
	CHECK_VK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr));
	ASSERTION(presentModeCount > 0);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	CHECK_VK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data()));

	// Prefer mailbox mode if present, it's the lowest latency non-tearing present  mode
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < presentModeCount; i++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	//Get width and height
	m_width = surfCap.currentExtent.width;
	m_height = surfCap.currentExtent.height;

	//Get how many images in the surface
	m_swapchainImgCount = surfCap.minImageCount + 1 < surfCap.maxImageCount ? surfCap.minImageCount + 1 : surfCap.maxImageCount;

	//Don't know what's this
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfCap.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfCap.currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = m_surface;
	swapchainCreateInfo.minImageCount = m_swapchainImgCount;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainCreateInfo.clipped = true;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageExtent.width = m_width;
	swapchainCreateInfo.imageExtent.height = m_height;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.imageFormat = m_surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = m_surfaceFormat.colorSpace;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	CHECK_VK_ERROR(vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain));
}

void VulkanInstance::Update()
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
		//render();
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

void VulkanInstance::InitCommandPool()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = m_graphicQueueIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	CHECK_VK_ERROR(vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool));
}

void VulkanInstance::InitSetupCommandBuffer()
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = m_commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	CHECK_VK_ERROR(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_setupCommandBuffer));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CHECK_VK_ERROR(vkBeginCommandBuffer(m_setupCommandBuffer, &beginInfo));
}

void VulkanInstance::InitSwapchainImgs()
{
	m_swapchainImg.images.resize(m_swapchainImgCount);
	m_swapchainImg.views.resize(m_swapchainImgCount);
	uint32_t count = 0;
	CHECK_VK_ERROR(vkGetSwapchainImagesKHR(m_device, m_swapchain, &count, nullptr));
	ASSERTION(count == m_swapchainImgCount);
	CHECK_VK_ERROR(vkGetSwapchainImagesKHR(m_device, m_swapchain, &count, m_swapchainImg.images.data()));
	
	for (uint32_t i = 0; i < m_swapchainImgCount; i++)
	{
		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = 0;
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
	}

	for (uint32_t i = 0; i < m_swapchainImgCount; i++)
	{
		//Create image view
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.format = m_surfaceFormat.format;
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
		CHECK_VK_ERROR(vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_swapchainImg.views[i]));

		//Change image layout
		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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

void VulkanInstance::InitDepthStencil()
{
	//create image of depth stencil
	VkImageCreateInfo dsCreateInfo = {};
	dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	dsCreateInfo.format = m_depthStencil.format;
	dsCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	dsCreateInfo.arrayLayers = 1;
	dsCreateInfo.extent.depth = 1;
	dsCreateInfo.extent.width = m_width;
	dsCreateInfo.extent.height = m_height;
	dsCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	dsCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	dsCreateInfo.mipLevels = 1;
	dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	CHECK_VK_ERROR(vkCreateImage(m_device, &dsCreateInfo, nullptr, &m_depthStencil.image));

	//Get memory requirements from depth stencil image
	VkMemoryRequirements dsReq = {};
	vkGetImageMemoryRequirements(m_device, m_depthStencil.image, &dsReq);
	
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = dsReq.size;
	
	uint32_t typeBits = dsReq.memoryTypeBits;
	uint32_t typeIndex = 0;
	while (typeBits)
	{
		if (typeBits & 1)
		{
			if (m_physicalDeviceMemoryProperties.memoryTypes[typeIndex].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				memoryAllocateInfo.memoryTypeIndex = typeIndex;
				break;
			}
		}
		typeBits >>= 1;
		typeIndex++;
	}

	CHECK_VK_ERROR(vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_depthStencil.memory));

	//Bind memory to depth stencil image
	CHECK_VK_ERROR(vkBindImageMemory(m_device, m_depthStencil.image, m_depthStencil.memory, 0));

	//Create depth stencil image view
	VkImageViewCreateInfo dsImageViewCreateInfo = {};
	dsImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	dsImageViewCreateInfo.image = m_depthStencil.image;
	dsImageViewCreateInfo.format = m_depthStencil.format;
	dsImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	dsImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	dsImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	dsImageViewCreateInfo.subresourceRange.layerCount = 1;
	dsImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	dsImageViewCreateInfo.subresourceRange.levelCount = 1;

	CHECK_VK_ERROR(vkCreateImageView(m_device, &dsImageViewCreateInfo, nullptr, &m_depthStencil.view));

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = m_depthStencil.image;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barrier.dstQueueFamilyIndex = m_graphicQueueIndex;
	barrier.srcAccessMask = 0;
	barrier.srcQueueFamilyIndex = m_graphicQueueIndex;
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

void VulkanInstance::InitRenderpass()
{
	std::vector<VkAttachmentDescription> attachmentDescs(2);
	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescs[0].format = m_surfaceFormat.format;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[1].format = m_depthStencil.format;
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

	CHECK_VK_ERROR(vkCreateRenderPass(m_device, &renderpassCreateInfo, nullptr, &m_renderpass));
}

void VulkanInstance::InitFrameBuffer()
{
	m_framebuffers.resize(m_swapchainImgCount);
	std::vector<VkImageView> attachments(2);
	attachments[1] = m_depthStencil.view;

	for (uint32_t i = 0; i < m_swapchainImgCount; i++)
	{
		attachments[0] = m_swapchainImg.views[i];

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.attachmentCount = 2;
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.width = m_width;
		framebufferCreateInfo.height = m_height;
		framebufferCreateInfo.renderPass = m_renderpass;

		CHECK_VK_ERROR(vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &m_framebuffers[i]));
	}
}

void VulkanInstance::InitVertices()
{
	float vertices[] =
	{
		-1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
		1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 1.0, 0.0, 0.0
	};

	int32_t indices[] = { 0, 1, 2 };

	Buffer stageVertexBuffer, stageIndexBuffer;


	//Create staging vertex buffer
	stageVertexBuffer.info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stageVertexBuffer.info.size = sizeof(vertices);
	stageVertexBuffer.info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	CHECK_VK_ERROR(vkCreateBuffer(m_device, &stageVertexBuffer.info, nullptr, &stageVertexBuffer.buffer));

	vkGetBufferMemoryRequirements(m_device, stageVertexBuffer.buffer, &stageVertexBuffer.reqs);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = stageVertexBuffer.reqs.size;

	uint32_t typeIndex = 0;
	uint32_t typeBits = stageVertexBuffer.reqs.memoryTypeBits;
	while (typeBits)
	{
		if (typeBits & 1)
		{
			if (m_physicalDeviceMemoryProperties.memoryTypes[typeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				allocateInfo.memoryTypeIndex = typeIndex;
				break;
			}
		}
		typeBits >>= 1;
		typeIndex++;
	}

	CHECK_VK_ERROR(vkAllocateMemory(m_device, &allocateInfo, nullptr, &stageVertexBuffer.memory));
	CHECK_VK_ERROR(vkBindBufferMemory(m_device, stageVertexBuffer.buffer, stageVertexBuffer.memory, 0));

	void* pData;
	CHECK_VK_ERROR(vkMapMemory(m_device, stageVertexBuffer.memory, 0, stageVertexBuffer.reqs.size, 0, &pData));
	memcpy(pData, vertices, stageVertexBuffer.reqs.size);
	vkUnmapMemory(m_device, stageVertexBuffer.memory);


	//Create vertex buffer
	m_vertexBuffer.info.size = sizeof(vertices);
	m_vertexBuffer.info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	CHECK_VK_ERROR(vkCreateBuffer(m_device, &m_vertexBuffer.info, nullptr, &m_vertexBuffer.buffer));
	vkGetBufferMemoryRequirements(m_device, m_vertexBuffer.buffer, &m_vertexBuffer.reqs);

	allocateInfo.allocationSize = m_vertexBuffer.reqs.size;

	typeIndex = 0;
	typeBits = m_vertexBuffer.reqs.memoryTypeBits;
	while (typeBits)
	{
		if (typeBits & 1)
		{
			if (m_physicalDeviceMemoryProperties.memoryTypes[typeIndex].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				allocateInfo.memoryTypeIndex = typeIndex;
				break;
			}
		}
		typeIndex++;
		typeBits >>= 1;
	}

	CHECK_VK_ERROR(vkAllocateMemory(m_device, &allocateInfo, nullptr, &m_vertexBuffer.memory));
	CHECK_VK_ERROR(vkBindBufferMemory(m_device, m_vertexBuffer.buffer, m_vertexBuffer.memory, 0));


	//Create staging index buffer
	stageIndexBuffer.info.size = sizeof(indices);
	stageIndexBuffer.info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	CHECK_VK_ERROR(vkCreateBuffer(m_device, &stageIndexBuffer.info, nullptr, &stageIndexBuffer.buffer));
	vkGetBufferMemoryRequirements(m_device, stageIndexBuffer.buffer, &stageIndexBuffer.reqs);

	allocateInfo.allocationSize = stageIndexBuffer.reqs.size;

	typeIndex = 0;
	typeBits = stageIndexBuffer.reqs.memoryTypeBits;
	while (typeBits)
	{
		if (typeBits & 1)
		{
			if (m_physicalDeviceMemoryProperties.memoryTypes[typeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				allocateInfo.memoryTypeIndex = typeIndex;
				break;
			}
		}
		typeIndex++;
		typeBits >>= 1;
	}

	CHECK_VK_ERROR(vkAllocateMemory(m_device, &allocateInfo, nullptr, &stageIndexBuffer.memory));
	CHECK_VK_ERROR(vkBindBufferMemory(m_device, stageIndexBuffer.buffer, stageIndexBuffer.memory, 0));

	CHECK_VK_ERROR(vkMapMemory(m_device, stageIndexBuffer.memory, 0, stageIndexBuffer.reqs.size, 0, &pData));
	memcpy(pData, indices, stageIndexBuffer.reqs.size);
	vkUnmapMemory(m_device, stageIndexBuffer.memory);


	//Create index buffer
	m_indexBuffer.info.size = sizeof(indices);
	m_indexBuffer.info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	CHECK_VK_ERROR(vkCreateBuffer(m_device, &m_indexBuffer.info, nullptr, &m_indexBuffer.buffer));
	vkGetBufferMemoryRequirements(m_device, m_indexBuffer.buffer, &m_indexBuffer.reqs);

	allocateInfo.allocationSize = m_indexBuffer.reqs.size;

	typeIndex = 0;
	typeBits = m_indexBuffer.reqs.memoryTypeBits;
	while (typeBits)
	{
		if (typeBits & 1)
		{
			if (m_physicalDeviceMemoryProperties.memoryTypes[typeIndex].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				allocateInfo.memoryTypeIndex = typeIndex;
				break;
			}
		}
		typeIndex++;
		typeBits >>= 1;
	}

	CHECK_VK_ERROR(vkAllocateMemory(m_device, &allocateInfo, nullptr, &m_indexBuffer.memory));
	CHECK_VK_ERROR(vkBindBufferMemory(m_device, m_indexBuffer.buffer, m_indexBuffer.memory, 0));

	//Setup a barrier for staging buffers
	VkBufferMemoryBarrier barriers[2] = {};

	barriers[0] = {};
	barriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barriers[0].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barriers[0].buffer = stageVertexBuffer.buffer;
	barriers[0].offset = 0;
	barriers[0].size = stageVertexBuffer.info.size;

	barriers[1] = {};
	barriers[1].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barriers[1].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barriers[1].buffer = stageIndexBuffer.buffer;
	barriers[1].offset = 0;
	barriers[1].size = stageIndexBuffer.info.size;

	vkCmdPipelineBarrier(m_setupCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		2, &barriers[0],
		0, nullptr);

	//Transfer data from staging buffers to device local buffers
	//Copy vertex buffer
	VkBufferCopy copy = {};
	copy.dstOffset = 0;
	copy.srcOffset = 0;
	copy.size = stageVertexBuffer.info.size;
	vkCmdCopyBuffer(m_setupCommandBuffer, stageVertexBuffer.buffer, m_vertexBuffer.buffer, 1, &copy);

	//Copy index buffer
	copy.size = stageIndexBuffer.info.size;
	vkCmdCopyBuffer(m_setupCommandBuffer, stageIndexBuffer.buffer, m_indexBuffer.buffer, 1, &copy);

	//Setup a barrier for memory changes
	barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barriers[0].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;	
	barriers[0].buffer = m_vertexBuffer.buffer;

	barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barriers[1].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;	//not sure if it's the same as vertex buffer
	barriers[1].buffer = m_indexBuffer.buffer;

	vkCmdPipelineBarrier(m_setupCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		2, &barriers[0],
		0, nullptr);

	//Binding and attributes information
	m_vertexBuffer.bindingDesc.binding = 0;		//hard coded 0
	m_vertexBuffer.bindingDesc.stride = sizeof(float) * 6;	//xyzrgb, all hard coded, mockup code, don't care, just for learning
	m_vertexBuffer.bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	m_vertexBuffer.attribDesc.resize(2);
	m_vertexBuffer.attribDesc[0].binding = 0;
	m_vertexBuffer.attribDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_vertexBuffer.attribDesc[0].location = 0;	//layout location 0 in shader
	m_vertexBuffer.attribDesc[0].offset = 0;

	m_vertexBuffer.attribDesc[1].binding = 0;
	m_vertexBuffer.attribDesc[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	m_vertexBuffer.attribDesc[1].location = 1;
	m_vertexBuffer.attribDesc[1].offset = sizeof(float) * 3;	//after xyz
}

void VulkanInstance::InitUniforms()
{
	//Create uniform buffer
	m_uniformBuffer.info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	m_uniformBuffer.info.size = sizeof(MVP);
	CHECK_VK_ERROR(vkCreateBuffer(m_device, &m_uniformBuffer.info, nullptr, &m_uniformBuffer.buffer));

	vkGetBufferMemoryRequirements(m_device, m_uniformBuffer.buffer, &m_uniformBuffer.reqs);
	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = m_uniformBuffer.reqs.size;

	uint32_t typeIndex = 0;
	uint32_t typeBits = m_uniformBuffer.reqs.memoryTypeBits;
	while (typeBits)
	{
		if (typeBits & 1)
		{
			if (m_physicalDeviceMemoryProperties.memoryTypes[typeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				allocateInfo.memoryTypeIndex = typeIndex;
				break;
			}
		}
		typeIndex++;
		typeBits >>= 1;
	}

	CHECK_VK_ERROR(vkAllocateMemory(m_device, &allocateInfo, nullptr, &m_uniformBuffer.memory));
	CHECK_VK_ERROR(vkBindBufferMemory(m_device, m_uniformBuffer.buffer, m_uniformBuffer.memory, 0));

	memset(m_mvp.model, 0, sizeof(m_mvp.model));
	memset(m_mvp.view, 0, sizeof(m_mvp.view));
	memset(m_mvp.projection, 0, sizeof(m_mvp.projection));
	m_mvp.model[0] = m_mvp.model[5] = m_mvp.model[10] = m_mvp.model[15] = 1.0f;
	m_mvp.view[0] = m_mvp.view[5] = m_mvp.view[10] = m_mvp.view[15] = 1.0f;
	m_mvp.projection[0] = m_mvp.projection[5] = m_mvp.projection[10] = m_mvp.projection[15] = 1.0f;

	void* pData;
	CHECK_VK_ERROR(vkMapMemory(m_device, m_uniformBuffer.memory, 0, m_uniformBuffer.info.size, 0, &pData));
	memcpy(pData, &m_mvp, sizeof(MVP));
	vkUnmapMemory(m_device, m_uniformBuffer.memory);

	m_mvp.modelDescriptor.buffer = m_uniformBuffer.buffer;
	m_mvp.modelDescriptor.offset = 0;
	m_mvp.modelDescriptor.range = sizeof(m_mvp.model);

	m_mvp.viewDescriptor.buffer = m_uniformBuffer.buffer;
	m_mvp.viewDescriptor.offset = m_mvp.modelDescriptor.offset + m_mvp.modelDescriptor.range;
	m_mvp.viewDescriptor.range = sizeof(m_mvp.view);

	m_mvp.projDescriptor.buffer = m_uniformBuffer.buffer;
	m_mvp.projDescriptor.offset = m_mvp.viewDescriptor.offset + m_mvp.viewDescriptor.range;
	m_mvp.projDescriptor.range = sizeof(m_mvp.projection);

	//Setup a barrier to let shader know its latest updated information in buffer
	VkBufferMemoryBarrier barrier = {};
	barrier.buffer = m_uniformBuffer.buffer;
	barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
	barrier.offset = 0;
	barrier.size = sizeof(MVP);
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	
	vkCmdPipelineBarrier(m_setupCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		1, &barrier,
		0, nullptr);
}

void VulkanInstance::InitDescriptorSetLayout()
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

		{
			1,	//binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	//type
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		},

		{
			2,	//binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	//type
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		}
	};

	VkDescriptorSetLayoutCreateInfo dslayoutInfo = {};
	dslayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dslayoutInfo.bindingCount = m_dsLayoutBinding.size();
	dslayoutInfo.pBindings = m_dsLayoutBinding.data();
	CHECK_VK_ERROR(vkCreateDescriptorSetLayout(m_device, &dslayoutInfo, nullptr, &m_descriptorSetLayout));

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	CHECK_VK_ERROR(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
}

void VulkanInstance::InitPipeline()
{
	VkPipelineColorBlendAttachmentState blendState = {};
	blendState.colorWriteMask = 0xf;
	blendState.blendEnable = VK_TRUE;

	blendState.colorBlendOp = VK_BLEND_OP_ADD;
	blendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

	blendState.alphaBlendOp = VK_BLEND_OP_ADD;
	blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

	VkPipelineColorBlendStateCreateInfo blendStateInfo = {};
	blendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendStateInfo.logicOpEnable = VK_TRUE;
	blendStateInfo.attachmentCount = 1;
	blendStateInfo.pAttachments = &blendState;

	VkPipelineDepthStencilStateCreateInfo dsCreateInfo = {};
	dsCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	dsCreateInfo.depthTestEnable = VK_TRUE;
	dsCreateInfo.depthWriteEnable = VK_TRUE;
	dsCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;

	VkPipelineInputAssemblyStateCreateInfo assCreateinfo = {};
	assCreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assCreateinfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pColorBlendState = &blendStateInfo;
	pipelineInfo.pDepthStencilState = &dsCreateInfo;
	pipelineInfo.pInputAssemblyState = &assCreateinfo;
}