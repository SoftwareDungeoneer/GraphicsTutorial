#ifndef GDI_FONT_WINDOW_H
#define GDI_FONT_WINDOW_H
#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "tchar.h"

#include <map>
#include <memory>
#include <vector>

class GDIFontWindow
{
public:
	GDIFontWindow();
	~GDIFontWindow();

	HRESULT Create();

protected:
	HWND hWnd{ 0 };
	HFONT hFont{ 0 };
	HBITMAP hBitmap{ 0 };
	long bmpCx{ 128 }, bmpCy{ 128 };
	TEXTMETRIC fontMetrics;

	BITMAPINFO bitmapInfo;
	std::vector<BYTE> DIBits;
	std::map<TCHAR, RECT> glyphQuads;

private:
	GDIFontWindow(GDIFontWindow&&) = delete;
	GDIFontWindow(const GDIFontWindow&) = delete;
	GDIFontWindow& operator=(GDIFontWindow&&) = delete;
	GDIFontWindow& operator=(const GDIFontWindow&) = delete;

	void RegisterWindowClass();
	HRESULT CreateUIWindow();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM, LPARAM);
	LRESULT OnCreate();
	LRESULT OnDestroy();
	LRESULT OnSize();
	LRESULT OnPaint();

	static LPCTSTR kWindowClassName;
};

#endif // GDI_FONT_WINDOW_H
