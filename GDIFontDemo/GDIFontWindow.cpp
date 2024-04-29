#include "GDIFontWindow.h"

#include <tchar.h>

LPCTSTR GDIFontWindow::kWindowClassName{ _T("GDI Font Window") };

namespace
{
	LPCTSTR kClassPointerProp{ _T("Class Pointer") };

	LPCTSTR kGlyphChars =
		_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
		_T("abcdefghijklmnopqrstuvwxyz")
		_T("0123456789-=!@#$%^&*()_+`")
		_T("[]\\{}|;:'\",./<>?");
}

GDIFontWindow::GDIFontWindow()
{
}

GDIFontWindow::~GDIFontWindow()
{
}

HRESULT GDIFontWindow::Create()
{
	RegisterWindowClass();
	return CreateUIWindow();
}

void GDIFontWindow::RegisterWindowClass()
{
	WNDCLASSEX wc{
		sizeof(WNDCLASSEX),
		CS_HREDRAW | CS_VREDRAW,
		&GDIFontWindow::WndProc,
		0, 0,
		GetModuleHandle(NULL),
		LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW),
		((HBRUSH)(COLOR_WINDOW + 1)),
		nullptr,
		kWindowClassName,
		NULL
	};
	RegisterClassEx(&wc);
}

HRESULT GDIFontWindow::CreateUIWindow()
{
	hWnd = CreateWindow(
		kWindowClassName,
		kWindowClassName,
		WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		400, 400,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		LPVOID(this)
	);
	if (!hWnd)
		return E_FAIL;
	return S_OK;
}

LRESULT CALLBACK GDIFontWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GDIFontWindow* Wnd{ nullptr };
	Wnd = static_cast<GDIFontWindow*>(GetProp(hWnd, kClassPointerProp));

	if (!Wnd && WM_CREATE == msg)
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		Wnd = static_cast<GDIFontWindow*>(lpcs->lpCreateParams);
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

		case WM_PAINT:
			return Wnd->OnPaint();
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT GDIFontWindow::OnCreate()
{
	HDC hdc = CreateDC(_T("DISPLAY"), nullptr, nullptr, nullptr);
	if (hdc)
	{
		HDC hCompatDC = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, bmpCx, bmpCy);
		int cap = GetDeviceCaps(hCompatDC, LOGPIXELSY);
		int md = MulDiv(14, cap, 72);
		hFont = CreateFont(
			MulDiv(14, GetDeviceCaps(hCompatDC, LOGPIXELSY), 72),
			0,
			0, 0,
			FW_DONTCARE,
			FALSE,
			FALSE,
			FALSE,
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY,
			FF_SWISS | VARIABLE_PITCH,
			_T("Calibri")
		);

		HBITMAP hBmpOld = (HBITMAP)SelectObject(hCompatDC, hBitmap);
		HFONT hFontOld = (HFONT)SelectObject(hCompatDC, hFont);

		if (hFont)
		{
			GetTextMetrics(hCompatDC, &fontMetrics);
		}

		BitBlt(hCompatDC, 0, 0, bmpCx, bmpCy, hCompatDC, 0, 0, BLACKNESS);

		SetBkColor(hCompatDC, RGB(0, 0, 0));
		SetTextColor(hCompatDC, RGB(0xff, 0xff, 0xff));
		SetTextAlign(hCompatDC, TA_TOP | TA_LEFT | TA_NOUPDATECP);

		long left{ 0 };
		long top{ 0 };
		long lineHeight{ 0 };
		for (unsigned n{ 0 }; kGlyphChars[n]; ++n)
		{
			SIZE size{ 0, 0 };
			GetTextExtentPoint32(hCompatDC, kGlyphChars + n, 1, &size);
			if (size.cy > lineHeight)
				lineHeight = size.cy;

			RECT r{ left, top, left + size.cx, top + size.cy };
			if (r.right >= bmpCx)
			{
				top += lineHeight;
				left = r.left = 0;
				r.right = size.cx;
				r.top += lineHeight;
				r.bottom += lineHeight;
				lineHeight = 0;
			}
			glyphQuads[kGlyphChars[n]] = r;

			left += size.cx;
			
			ExtTextOut(hCompatDC, r.left, r.top, 0, nullptr, kGlyphChars + n, 1, nullptr);
		}

		SelectObject(hCompatDC, hFontOld);
		SelectObject(hCompatDC, hBmpOld);
		DeleteDC(hCompatDC);

		memset(&bitmapInfo, 0, sizeof(bitmapInfo));
		bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

		int rc = GetDIBits(hdc, hBitmap, 0, 0, NULL, &bitmapInfo, DIB_RGB_COLORS);
		if (rc)
		{
			bitmapInfo.bmiHeader.biHeight = abs(bitmapInfo.bmiHeader.biHeight);
			bitmapInfo.bmiHeader.biCompression = BI_RGB;
			DIBits.resize(bitmapInfo.bmiHeader.biSizeImage);
			rc = GetDIBits(
				hdc,
				hBitmap, 
				0,
				bitmapInfo.bmiHeader.biHeight,
				DIBits.data(),
				&bitmapInfo,
				DIB_RGB_COLORS
			);
		}

		BITMAPFILEHEADER fileHeader{
			0x4d42,
			DWORD(sizeof(fileHeader) + bitmapInfo.bmiHeader.biSize + DIBits.size()),
			0,
			0,
			DWORD(sizeof(fileHeader) + bitmapInfo.bmiHeader.biSize)
		};

		HANDLE hFile = CreateFile(
			_T("TestOut.bmp"),
			GENERIC_WRITE,
			0,
			nullptr,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
			nullptr
		);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwWritten{ 0 };
			WriteFile(hFile, &fileHeader, sizeof(fileHeader), &dwWritten, nullptr);
			WriteFile(hFile, &bitmapInfo, bitmapInfo.bmiHeader.biSize, &dwWritten, nullptr);
			WriteFile(hFile, DIBits.data(), (DWORD)DIBits.size(), &dwWritten, nullptr);
			CloseHandle(hFile);
		}

	}
	DeleteDC(hdc);
	return 0;
}

LRESULT GDIFontWindow::OnDestroy()
{
	hWnd = nullptr;
	PostQuitMessage(0);
	return 0;
}

LRESULT GDIFontWindow::OnSize()
{
	return 0;
}

LRESULT GDIFontWindow::OnPaint()
{
	RECT rClient;
	GetClientRect(hWnd, &rClient);

	PAINTSTRUCT ps;
	BeginPaint(hWnd, &ps);

	HDC hCompatDC = CreateCompatibleDC(ps.hdc);
	HBITMAP hBmpOld = (HBITMAP)SelectObject(hCompatDC, hBitmap);

	unsigned left = (rClient.right - rClient.left - bmpCx) / 2;
	unsigned top = (rClient.bottom - rClient.top - bmpCy) / 2;
	BitBlt(ps.hdc, left, top, bmpCx, bmpCy, hCompatDC, 0, 0, SRCCOPY);

	SelectObject(hCompatDC, hBmpOld);
	DeleteDC(hCompatDC);
	EndPaint(hWnd, &ps);

	return 0;
}
