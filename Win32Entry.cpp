#include <windows.h>
#include "vulkan\VulkanGLobal.h"
#include "scene/SceneGenerator.h"

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
	VulkanGlobal::GetInstance()->InitVulkan(hInstance, WndProc);
	VulkanGlobal::GetInstance()->Update();
	SceneGenerator::Free();
	VulkanGlobal::Free();
	GlobalDeviceObjects::GetInstance()->Free();
	return 0;
}

