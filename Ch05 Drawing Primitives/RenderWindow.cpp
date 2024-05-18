#include "RenderWindow.h"

#include <tchar.h>

#include <cassert>
#include <array>
#include <map>
#include <random>
#include <sstream>
#include <vector>

#include "AppLocalMessages.h"
#include "Renderer.h"
#include "Ch03_FirstTriangle.h"
#include "Ch04_MovingTriangles.h"
#include "Ch05_DrawingPrimitives.h"
#include "Ch06_PolyLines.h"
#include "Ch07_Textures.h"
#include "Ch08_FontAtlas.h"
#include "Ch09_DepthBuffer.h"
#include "Ch10_KernelsAndFilters.h"

#include "util.h"

#include "resource.h"

LPCTSTR RenderWindow::kWindowClassName{ _T("Render Window") };

namespace
{
	LPCTSTR kClassPointerProp{ _T("Class Pointer") };

	std::string RectToString(const RECT& r)
	{
		std::ostringstream oss;
		oss << r;
		return oss.str();
	}
}

RenderWindow::RenderWindow(std::shared_ptr<Settings> _s) :appSettings(_s)
{
	debugDataStore = std::make_shared<DebugDataWindow::DataBlock>();
}

RenderWindow::~RenderWindow()
{}

HRESULT RenderWindow::Create()
{
	RegisterWindowClass();
	return CreateUIWindow();
}

void RenderWindow::Update(double interval)
{
	if (activeRenderer)
	{
		activeRenderer->Update(interval);
		activeRenderer->Render();
	}
}

void RenderWindow::RegisterWindowClass()
{
	WNDCLASSEX wndClass{
		sizeof(WNDCLASSEX),
		0,
		&RenderWindow::WndProc,
		0,
		0,
		GetModuleHandle(NULL),
		LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW),
		nullptr,
		MAKEINTRESOURCE(IDM_MAIN_MENU),
		kWindowClassName,
		nullptr
	};

	RegisterClassEx(&wndClass);
}

HRESULT RenderWindow::CreateUIWindow()
{
	DWORD window_styles = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	
	RECT requested;
	AdjustWindowRect(&requested, window_styles, TRUE);

	auto [cx, cy] = appSettings->mainWindowSize;
	if (cx == CW_USEDEFAULT) cx = requested.right - requested.left;
	if (cy == CW_USEDEFAULT) cy = requested.bottom - requested.top;
	hWnd = CreateWindow(
		kWindowClassName,
		kWindowClassName,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		appSettings->mainWindowPos.x,
		appSettings->mainWindowPos.y,
		cx,
		cy,
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		(LPVOID)this
	);

	if (!hWnd)
		return MAKE_HRESULT(1, 0x20FF, 1);

	return S_OK;
}

void RenderWindow::UpdateDebugInfo(const std::string& key, const std::string& value)
{
	const std::string section{ "Render Window" };
	if (debugDataWindow)
	{
		debugDataWindow->UpdateValue(section, key, value);
	}
	else
	{
		(*debugDataStore)[section].first[key] = value;
	}
}

void RenderWindow::UpdatePosition()
{
	RECT rWnd;
	RECT rClient;
	GetWindowRect(hWnd, &rWnd);
	GetClientRect(hWnd, &rClient);
	std::ostringstream oss;
	auto windowWidth = rWnd.right - rWnd.left;
	auto windowHeight = rWnd.bottom - rWnd.top;

	appSettings->mainWindowPos = { rWnd.left, rWnd.top };
	appSettings->mainWindowSize = { windowWidth, windowHeight };

	oss << rWnd << "(" << windowWidth << " x " << windowHeight << ")";
	UpdateDebugInfo("Window Position", oss.str());

	oss.str("");
	oss << rClient.right << " x " << rClient.bottom;
	UpdateDebugInfo("Client size", oss.str());
}
void RenderWindow::SetDemo(Demos demo)
{
	assert(demo < Demos::COUNT);
	using PtrT = std::shared_ptr<Renderer>; 
	struct Thunk { void (*fn)(PtrT&, HWND); };
	constexpr Thunk thunks[] = {
		{ [](PtrT& ptr, HWND h) -> void { ptr = std::make_shared<FirstTriangle>(h); } },
		{ [](PtrT& ptr, HWND h) -> void { ptr = std::make_shared<MovingTriangles>(h); } },
		{ [](PtrT& ptr, HWND h) -> void { ptr = std::make_shared<DrawingPrimitives>(h); } },
		{ [](PtrT& ptr, HWND h) -> void { ptr = std::make_shared<PolyLines>(h); } },
		{ [](PtrT& ptr, HWND h) -> void { ptr = std::make_shared<Textures>(h); } },
		{ [](PtrT& ptr, HWND h) -> void { ptr = std::make_shared<FontAtlas>(h); } },
		{ [](PtrT& ptr, HWND h) -> void { ptr = std::make_shared<DepthBuffer>(h); }},
		{ [](PtrT& ptr, HWND h) -> void { ptr = std::make_shared<KernelFilters>(h); }}
	};
	static_assert(countof(thunks) == static_cast<unsigned>(Demos::COUNT));
	if (demo < Demos::COUNT)
	{
		activeRenderer.reset();
		thunks[static_cast<unsigned>(demo)].fn(activeRenderer, hWnd);
	}
}

LRESULT CALLBACK RenderWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RenderWindow* Wnd{ nullptr };
	Wnd = static_cast<RenderWindow*>(GetProp(hWnd, kClassPointerProp));

	if (!Wnd && WM_CREATE == msg)
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		Wnd = static_cast<RenderWindow*>(lpcs->lpCreateParams);
		SetProp(hWnd, kClassPointerProp, static_cast<HANDLE>(Wnd));
		Wnd->hWnd = hWnd;
	}

	if (Wnd)
	{
		switch (msg)
		{
		case WM_CREATE:
			return Wnd->OnCreate();

		case WM_COMMAND:
			return Wnd->OnCommand(wParam, lParam);

		case WM_MOVE:
			return Wnd->OnMove();

		case WM_SIZE:
			return Wnd->OnSize();

		case WM_DESTROY:
			return Wnd->OnDestroy();

		case AM_DEBUGDATA_DESTROYED:
			return Wnd->OnNotifyDebugDataDestroyed();
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT RenderWindow::OnCreate()
{
	toolWindow = std::make_shared<ToolWindow>(this, appSettings);
	if (toolWindow)
		toolWindow->Create();
	debugDataWindow = std::make_shared<DebugDataWindow>(hWnd, debugDataStore, appSettings);
	if (debugDataWindow)
		debugDataWindow->Create();

	return 0;
}

LRESULT RenderWindow::OnDestroy()
{
	hWnd = nullptr;
	PostQuitMessage(0);
	return 0;
}

LRESULT RenderWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	WPARAM ctrlCode = HIWORD(wParam);
	WPARAM id = LOWORD(wParam);
	[[maybe_unused]] HWND hCtrl = ((HWND)lParam);

	switch (id)
	{
	case IDM_FILE_EXIT:
		return OnFileExit();

	case IDM_VIEW_SETTINGS:
		return OnViewSettings();
	}

	return DefWindowProc(hWnd, WM_COMMAND, wParam, lParam);
}

LRESULT RenderWindow::OnMove()
{
	UpdatePosition();
	return 0;
}

LRESULT RenderWindow::OnSize()
{
	RECT rClient;
	GetClientRect(hWnd, &rClient);
	windowWidth = rClient.right - rClient.left;
	windowHeight = rClient.bottom - rClient.top;

	if (activeRenderer)
		activeRenderer->Resize(windowWidth, windowHeight);

	UpdatePosition();

	return 0;
}

LRESULT RenderWindow::OnNotifyDebugDataDestroyed()
{
	debugDataWindow.reset();
	return 0;
}

LRESULT RenderWindow::OnFileExit()
{
	PostMessage(hWnd, WM_DESTROY, 0, 0);
	return 0;
}

LRESULT RenderWindow::OnViewSettings()
{
	MessageBox(hWnd, _T("No settings to show"), _T("Settings"), MB_OK);
	return 0;
}
