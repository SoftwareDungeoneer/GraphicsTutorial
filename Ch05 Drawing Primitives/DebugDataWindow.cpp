#include "DebugDataWindow.h"

#include <tchar.h>

#include "AppLocalMessages.h"

LPCTSTR DebugDataWindow::kWindowClassName{ _T("Debug Data Window") };

namespace
{
	LPCTSTR kClassPointerProp{ _T("Class Pointer") };
}

DebugDataWindow::DebugDataWindow(HWND rw, std::shared_ptr<DataBlock> _db) :
	renderWindow(rw),
	valueMap(_db)
{
}

DebugDataWindow::~DebugDataWindow()
{
	if (hWnd)
	{
		RemoveProp(hWnd, kClassPointerProp);
		DestroyWindow(hWnd);
	}
}

void DebugDataWindow::UpdateWindow()
{
	InvalidateRect(hWnd, nullptr, TRUE);
}

void DebugDataWindow::UpdateValue(
	const std::string& section,
	const std::string& key,
	const std::string& value
)
{
	(*valueMap)[section].first[key] = value;
	UpdateWindow();
}

void DebugDataWindow::RemoveSection(const std::string& section)
{
	auto sectionIter = valueMap->find(section);
	if (sectionIter != valueMap->end())
	{
		valueMap->erase(sectionIter);
	}
	UpdateWindow();
}

void DebugDataWindow::RemoveValue(const std::string& section, const std::string& key)
{
	auto sectionIter = valueMap->find(section);
	if (sectionIter != valueMap->end())
	{
		auto& kvMap = sectionIter->second.first;
		auto keyIter = kvMap.find(key);
		if (keyIter != kvMap.end())
		{
			kvMap.erase(keyIter);
		}
	}
	UpdateWindow();
}

HRESULT DebugDataWindow::Create()
{
	RegisterWindowClass();
	return CreateUIWindow();
}

void DebugDataWindow::RegisterWindowClass()
{
	WNDCLASSEX wndClass{
		sizeof(WNDCLASSEX),
		CS_HREDRAW | CS_VREDRAW,
		&DebugDataWindow::WndProc,
		0,
		0,
		GetModuleHandle(NULL),
		LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW),
		((HBRUSH)(COLOR_WINDOW + 1)),
		nullptr,
		kWindowClassName,
		nullptr
	};

	RegisterClassEx(&wndClass);
}

HRESULT DebugDataWindow::CreateUIWindow()
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

LRESULT CALLBACK DebugDataWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DebugDataWindow* Wnd{ nullptr };
	Wnd = static_cast<DebugDataWindow*>(GetProp(hWnd, kClassPointerProp));

	if (!Wnd && WM_CREATE == msg)
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		Wnd = static_cast<DebugDataWindow*>(lpcs->lpCreateParams);
		SetProp(hWnd, kClassPointerProp, static_cast<HANDLE>(Wnd));
		Wnd->hWnd = hWnd;
	}

	if (Wnd)
	{
		switch (msg)
		{
		case WM_CREATE:
			return Wnd->OnCreate();

		case WM_DESTROY:
			return Wnd->OnDestroy();

		case WM_PAINT:
			return Wnd->OnPaint();
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT DebugDataWindow::OnCreate()
{
	return 0;
}

LRESULT DebugDataWindow::OnDestroy()
{
	RemoveProp(hWnd, kClassPointerProp);
	hWnd = 0;
	SendMessage(renderWindow, AM_DEBUGDATA_DESTROYED, 0, 0);
	return 0;
}

LRESULT DebugDataWindow::OnPaint()
{
	RECT rClient;
	GetClientRect(hWnd, &rClient);

	PAINTSTRUCT ps;
	RECT rDraw{ rClient.left + 3, rClient.top + 3, rClient.right - 3, rClient.bottom - 3 };
	TEXTMETRICA tm;
	BeginPaint(hWnd, &ps);
	BitBlt(ps.hdc, 0, 0, rClient.right, rClient.bottom, ps.hdc, 0, 0, WHITENESS);
	GetTextMetricsA(ps.hdc, &tm);
	SetTextAlign(ps.hdc, TA_TOP | TA_LEFT);

	int lineTop{ rDraw.top };
	constexpr int inset{ 10 };

	for (const auto& [section, dataMap] : *valueMap)
	{
		ExtTextOutA(
			ps.hdc, 
			rDraw.left,
			lineTop, 
			0,
			nullptr, 
			section.c_str(),
			static_cast<unsigned>(section.size()),
			nullptr);
		lineTop += tm.tmHeight;

		//if (!dataMap.second.expanded)
		//	continue;

		for (const auto& [key, value] : dataMap.first)
		{
			SetTextAlign(ps.hdc, TA_TOP | TA_LEFT);
			ExtTextOutA(
				ps.hdc,
				rDraw.left + inset,
				lineTop, 
				0, 
				nullptr, 
				key.c_str(),
				static_cast<unsigned>(key.size()),
				nullptr
			);

			SetTextAlign(ps.hdc, TA_TOP | TA_RIGHT);
			ExtTextOutA(
				ps.hdc,
				rDraw.right,
				lineTop, 
				0, 
				nullptr, 
				value.c_str(),
				static_cast<unsigned>(value.size()),
				nullptr
			);
			lineTop += tm.tmHeight;
		}
	}
	EndPaint(hWnd, &ps);
	return 0;
}
