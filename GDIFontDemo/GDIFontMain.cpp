#include <Windows.h>
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")

#include <tchar.h>

#include <type_traits>
#include <memory>

#include "GDIFontWindow.h"

constexpr TCHAR kWindowClassName[] = _T("Graphics Tutorial Window");

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE,
	LPSTR,
	int)
{
	//SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	std::shared_ptr<GDIFontWindow> mainWindow = std::make_shared<GDIFontWindow>();
	mainWindow->Create();

	LARGE_INTEGER qptLastUpdate;
	QueryPerformanceCounter(&qptLastUpdate);

	MSG msg;
	memset(&msg, 0, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	mainWindow.reset();
	CoUninitialize();
}
