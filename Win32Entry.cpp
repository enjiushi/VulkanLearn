#include <windows.h>
#include "vulkan\VulkanGLobal.h"

#if defined(_WIN32)
// Windows entry point
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	VulkanGlobal::GetInstance()->HandleMsg(hWnd, uMsg, wParam, lParam);
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
#endif
{
	VulkanGlobal::GetInstance()->InitVulkanInstance();
	VulkanGlobal::GetInstance()->InitPhysicalDevice();
	VulkanGlobal::GetInstance()->InitVulkanDevice();
	VulkanGlobal::GetInstance()->SetupWindow(hInstance, WndProc);
	VulkanGlobal::GetInstance()->InitSurface();
	VulkanGlobal::GetInstance()->InitSwapchain();
	VulkanGlobal::GetInstance()->InitQueue();

	VulkanGlobal::GetInstance()->InitCommandPool();
	VulkanGlobal::GetInstance()->InitSetupCommandBuffer();
	VulkanGlobal::GetInstance()->InitSwapchainImgs();
	VulkanGlobal::GetInstance()->InitDepthStencil();
	VulkanGlobal::GetInstance()->InitRenderpass();
	VulkanGlobal::GetInstance()->InitFrameBuffer();
	VulkanGlobal::GetInstance()->InitVertices();
	VulkanGlobal::GetInstance()->InitUniforms();
	VulkanGlobal::GetInstance()->InitDescriptorSetLayout();
	VulkanGlobal::GetInstance()->InitPipelineCache();
	VulkanGlobal::GetInstance()->InitPipeline();
	VulkanGlobal::GetInstance()->InitDescriptorPool();
	VulkanGlobal::GetInstance()->InitDescriptorSet();
	VulkanGlobal::GetInstance()->InitDrawCmdBuffers();
	VulkanGlobal::GetInstance()->InitSemaphore();
	VulkanGlobal::GetInstance()->EndSetup();

	VulkanGlobal::GetInstance()->Update();
}

