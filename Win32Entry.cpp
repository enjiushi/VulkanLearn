#include <windows.h>
#include "VulkanLearn/VulkanLearn.h"

#if defined(_WIN32)
// Windows entry point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
#endif
{
	VulkanInstance vulkanInst;
	vulkanInst.InitVulkanInstance();
}