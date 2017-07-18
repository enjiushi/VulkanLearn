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
	VulkanGlobal::GetInstance()->Init(hInstance, WndProc);
	VulkanGlobal::GetInstance()->Update();
	VulkanGlobal::Free();
	GlobalDeviceObjects::Free();
	return 0;
}

