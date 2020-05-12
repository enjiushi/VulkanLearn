#include <windows.h>
#include "appEntry\AppEntry.h"
#include "scene/SceneGenerator.h"

#if defined(_WIN32)
// Windows entry point
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	AppEntry::GetInstance()->HandleMsg(hWnd, uMsg, wParam, lParam);
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
#endif
{
	AppEntry::GetInstance()->InitVulkan(hInstance, WndProc);
	AppEntry::GetInstance()->Update();
	SceneGenerator::Free();
	AppEntry::Free();
	GlobalDeviceObjects::GetInstance()->Free();
	return 0;
}

