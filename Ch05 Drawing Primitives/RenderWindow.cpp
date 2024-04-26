#include "RenderWindow.h"

#include <tchar.h>

#include <array>
#include <map>
#include <random>
#include <vector>

#include "util.h"

LPCTSTR RenderWindow::kWindowClassName{ _T("Render Window") };

namespace
{
	LPCTSTR kClassPointerProp{ _T("Class Pointer") };
}

RenderWindow::RenderWindow()
{}

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
		nullptr,
		kWindowClassName,
		nullptr
	};

	RegisterClassEx(&wndClass);
}

HRESULT RenderWindow::CreateUIWindow()
{
	DWORD window_styles = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	RECT requested{ 0, 0, 800, 600 };
	AdjustWindowRect(&requested, window_styles, FALSE);
	int width = requested.right - requested.left;
	int height = requested.bottom - requested.top;

	hWnd = CreateWindow(
		kWindowClassName,
		kWindowClassName,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		(LPVOID)this
	);

	if (!hWnd)
		return MAKE_HRESULT(1, 0x20FF, 1);

	return S_OK;
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

		case WM_SIZE:
			return Wnd->OnSize();

		case WM_DESTROY:
			return Wnd->OnDestroy();
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT RenderWindow::OnCreate()
{
	return 0;
}

LRESULT RenderWindow::OnDestroy()
{
	hWnd = nullptr;
	PostQuitMessage(0);
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

	return 0;
}
