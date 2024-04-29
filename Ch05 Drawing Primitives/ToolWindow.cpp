#include "ToolWindow.h"

#include <Windows.h>
#include <tchar.h>

#include "RenderWindow.h"
#include "util.h"

namespace
{
	LPCTSTR kClassPointerProp{ _T("Class Pointer") };
	constexpr unsigned IDC_DEMO_SELECTOR{ 1001 };	
}

LPCTSTR ToolWindow::kWindowClassName = _T("Graphics Tutorial Tool Window");

const std::array<LPCTSTR, static_cast<size_t>(ToolWindow::eLabels::COUNT)> ToolWindow::labelStrings = {
	_T("Demo to run:"),
};

const std::array<LPCTSTR, static_cast<size_t>(RenderWindow::Demos::COUNT)> DemoStrings = {
	_T("First Triangle"),
	_T("Moving Triangles"),
	_T("Drawing Primitives"),
	_T("Antialiased Lines"),
	_T("Textured Quad"),
};

ToolWindow::ToolWindow(RenderWindow* rw):pRenderWindow(rw)
{
	ZeroInitialize(uiFontMetrics);
}

ToolWindow::~ToolWindow()
{}

void ToolWindow::Create()
{
	Register();
	CreateUIWindow();
}

void ToolWindow::Register()
{
	WNDCLASSEX wndClass{
		sizeof(WNDCLASSEX),
		0,
		&ToolWindow::WndProc,
		0,
		0,
		GetModuleHandle(NULL),
		LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW),
		((HBRUSH)(COLOR_APPWORKSPACE + 1)),
		nullptr,
		kWindowClassName,
		nullptr
	};

	RegisterClassEx(&wndClass);
}

void ToolWindow::CreateUIWindow()
{
	DWORD window_styles = WS_OVERLAPPED | WS_VISIBLE;
	RECT requested{ 0, 0, 250, 600 };
	AdjustWindowRect(&requested, window_styles, FALSE);
	int width = requested.right - requested.left;
	int height = requested.bottom - requested.top;

	hWnd = CreateWindow(
		kWindowClassName,
		kWindowClassName,
		window_styles,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		(LPVOID)this
	);
}

LRESULT CALLBACK ToolWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ToolWindow* Wnd{ nullptr };
	Wnd = static_cast<ToolWindow*>(GetProp(hWnd, kClassPointerProp));

	if (!Wnd && WM_CREATE == msg)
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		Wnd = static_cast<ToolWindow*>(lpcs->lpCreateParams);
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

		case WM_COMMAND:
			return Wnd->OnCommand(wParam, lParam);

		case WM_DESTROY:
			return Wnd->OnDestroy();

		case WM_PAINT:
			return Wnd->OnPaint();
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT ToolWindow::OnCreate()
{
	HDC hDC = CreateDC(_T("DISPLAY"), nullptr, nullptr, nullptr);
	if (hDC)
	{
		uiFont = CreateFont(
			-MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72),
			0,
			0, 0,
			FW_DONTCARE, 
			FALSE, 
			FALSE, 
			FALSE,
			ANSI_CHARSET, 
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			FF_SWISS | VARIABLE_PITCH,
			_T("Segoe UI")
		);

		if (uiFont)
		{
			GetTextMetrics(hDC, &uiFontMetrics);
		}
		DeleteDC(hDC);
	}

	DWORD dwSelectionBoxSyles = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE;
	dwSelectionBoxSyles |= CBS_DROPDOWNLIST;
	hDemoSelector = CreateWindow(
		_T("COMBOBOX"),
		_T(""),
		dwSelectionBoxSyles,
		5,
		5,
		250,
		250,
		hWnd,
		((HMENU)(IDC_DEMO_SELECTOR)),
		GetModuleHandle(NULL),
		nullptr
	);

	if (hDemoSelector)
	{
		if (uiFont)
			SendMessage(hDemoSelector, WM_SETFONT, reinterpret_cast<WPARAM>(uiFont), FALSE);

		for (unsigned n{ 0 }; n < static_cast<unsigned>(RenderWindow::Demos::COUNT); ++n)
		{
			SendMessage(hDemoSelector, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(DemoStrings[n]));
		}
		SendMessage(hDemoSelector, CB_SETCURSEL, 0, 0);

		if (pRenderWindow)
			pRenderWindow->SetDemo(static_cast<RenderWindow::Demos>(0));
	}
	return 0;
}

LRESULT ToolWindow::OnDestroy()
{
	return 0;
}

LRESULT ToolWindow::OnSize()
{
	RECT rClient;
	GetClientRect(hWnd, &rClient);

	const unsigned rightMargin{ 5 };
	const unsigned leftMargin{ 5 };

	unsigned verticalOffset = 5;

	// Do control layout
	
	// Demo label goes here
	labelPositions[static_cast<unsigned>(eLabels::DemoSelection)] = verticalOffset;
	verticalOffset += uiFontMetrics.tmHeight + 3;

	SetWindowPos(
		hDemoSelector, 
		NULL, 
		leftMargin, 
		verticalOffset, 
		rClient.right - leftMargin - rightMargin,
		250,
		SWP_NOOWNERZORDER
	);

	InvalidateRect(hWnd, &rClient, TRUE);
	return 0;
}

LRESULT ToolWindow::OnPaint()
{
	PAINTSTRUCT ps;
	BeginPaint(hWnd, &ps);

	HFONT hOldFont{ NULL };
	if (uiFont)
		hOldFont = (HFONT)SelectObject(ps.hdc, uiFont);

	SetBkMode(ps.hdc, TRANSPARENT);
	for (unsigned n{ 0 }; n < static_cast<unsigned>(eLabels::COUNT); ++n)
	{
		TextOut(ps.hdc, 5, labelPositions[n], labelStrings[n], _tcslen(labelStrings[n]));
	}

	SelectObject(ps.hdc, hOldFont);
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT ToolWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	unsigned cmd{ HIWORD(wParam) };
	unsigned ctrl{ LOWORD(wParam) };
	[[maybe_unused]] HWND hCtrl{ reinterpret_cast<HWND>(lParam) };

	switch (ctrl)
	{
	case IDC_DEMO_SELECTOR:
		return OnDemoSelector(cmd);

	default:
		break;
	}

	return DefWindowProc(hWnd, WM_COMMAND, wParam, lParam);
}

LRESULT ToolWindow::OnDemoSelector(unsigned command)
{
	LRESULT rVal{ 0 };
	switch (command)
	{
	case CBN_SELCHANGE:
		rVal = SendMessage(hDemoSelector, CB_GETCURSEL, 0, 0);
		if (pRenderWindow)
			pRenderWindow->SetDemo(static_cast<RenderWindow::Demos>(rVal));
		return 0;
	}

	return DefWindowProc(
		hWnd,
		WM_COMMAND,
		MAKEWPARAM(IDC_DEMO_SELECTOR, command),
		reinterpret_cast<LPARAM>(hDemoSelector)
	);
}
