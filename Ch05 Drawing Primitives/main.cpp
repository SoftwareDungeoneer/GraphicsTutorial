#include <Windows.h>

#include <tchar.h>

#include <type_traits>
#include <memory>

#include "RenderWindow.h"
#include "Ch05_DrawingPrimitives.h"
#include "Ch06_PolyLines.h"
#include "util.h"

#include "Mtx2x2.h"

constexpr TCHAR kWindowClassName[] = _T("Graphics Tutorial Window");

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE,
	LPSTR,
	int)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	std::shared_ptr<Settings> globalSettings = std::make_shared<Settings>(Settings::GetDefaultSettings());
	Settings::Deserialize(_T("config.ini"), globalSettings);

	LARGE_INTEGER qptLi;
	QueryPerformanceFrequency(&qptLi);
	double qptFrequency = 1.0 * qptLi.QuadPart;

	QueryPerformanceCounter(&qptLi);
	double qptStartup{ qptLi.QuadPart / qptFrequency };

	std::shared_ptr<RenderWindow> mainWindow = std::make_shared<RenderWindow>(globalSettings);
	mainWindow->Create();

	LARGE_INTEGER qptLastUpdate;
	QueryPerformanceCounter(&qptLastUpdate);

	MSG msg;
	ZeroInitialize(msg);
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		QueryPerformanceCounter(&qptLi);
		double interval = (qptLi.QuadPart - qptLastUpdate.QuadPart) / qptFrequency;
		mainWindow->Update(interval);
		qptLastUpdate = qptLi;
	}

	mainWindow.reset();

	Settings::Serialize(_T("config.ini"), globalSettings);
	CoUninitialize();
}
