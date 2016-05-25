#include <windows.h>
#include "VulkanLearn/VulkanLearn.h"

VulkanInstance* pVulkanInst;

#if defined(_WIN32)
// Windows entry point
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (pVulkanInst)
		pVulkanInst->HandleMsg(hWnd, uMsg, wParam, lParam);
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
#endif
{
	VulkanInstance vulkanInst;
	pVulkanInst = &vulkanInst;
	vulkanInst.InitVulkanInstance();
	vulkanInst.InitPhysicalDevice();
	vulkanInst.InitVulkanDevice();
	vulkanInst.SetupWindow(hInstance, WndProc);
	vulkanInst.InitSurface();
	vulkanInst.InitSwapchain();

	vulkanInst.InitCommandPool();
	vulkanInst.InitSetupCommandBuffer();
	vulkanInst.InitSwapchainImgs();
	vulkanInst.InitDepthStencil();
	vulkanInst.InitRenderpass();
	vulkanInst.InitFrameBuffer();
	vulkanInst.InitVertices();

	vulkanInst.Update();
}

