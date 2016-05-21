#include <windows.h>
#include "VulkanLearn/VulkanLearn.h"

#if defined(_WIN32)
// Windows entry point

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
#endif
{
	VulkanInstance vulkanInst;
	vulkanInst.InitVulkanInstance();
	vulkanInst.InitPhysicalDevice();
	vulkanInst.InitVulkanDevice();
	vulkanInst.InitSurface();
	vulkanInst.SetupWindow(hInstance, WndProc);
}

