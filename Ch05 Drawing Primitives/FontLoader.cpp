#include "FontLoader.h"

#include <tchar.h>

#include "util.h"

namespace
{
	LPCTSTR kGlyphChars =
		_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
		_T("abcdefghijklmnopqrstuvwxyz")
		_T("0123456789-=!@#$%^&*()_+`")
		_T("[]\\{}|;:'\",./<>?");
}
FontData FontLoader::LoadFont(LPCTSTR facename, int height)
{
	HFONT hFont{ 0 };
	HDC hdc = CreateDC(_T("DISPLAY"), nullptr, nullptr, nullptr);
	if (hdc)
	{
		hFont = CreateFont(
			height,
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

		if (!hFont)
		{
			return {};
		}
		DeleteDC(hdc);
	}

	return ConvertFontToAtlas(hFont);

}

FontData FontLoader::ConvertFontToAtlas(HFONT hFont)
{
	FontData out;
	memset(&out.tm, 0, sizeof(out.tm));
	memset(&out.bitmapInfo, 0, sizeof(out.bitmapInfo));

	HDC hDisplayDc = CreateDC(_T("DISPLAY"), nullptr, nullptr, nullptr);
	HDC hdc = CreateCompatibleDC(hDisplayDc);
	if (hdc)
	{
		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

		GetTextMetrics(hdc, &out.tm);

		bool succeeded{ false };
		int cxcy = 128;
		while (!succeeded)
		{
			// Presume succeess, so we can set failure later
			succeeded = true;

			long left{ 0 };
			long top{ 0 };
			for (unsigned n{ 0 }; kGlyphChars[n]; ++n)
			{
				SIZE size{ 0 , 0 };
				GetTextExtentPoint32(hdc, kGlyphChars + n, 1, &size);
				RECT r{ left, top, left + size.cx, top + size.cy };
				if (r.right >= cxcy)
				{
					top += out.tm.tmHeight;
					left = r.left = 0;
					r.right = size.cx;
					r.top = top;
					r.bottom = top + size.cy;
					if (r.bottom >= cxcy)
					{
						succeeded = false;
						cxcy <<= 1;
						break;
					}
				}
				out.glyphQuads[kGlyphChars[n]] = RectfFromRect(r);
				left += size.cx;
			}
		}

		// create & write to atlas
		HBITMAP hBitmap = CreateCompatibleBitmap(hDisplayDc, cxcy, cxcy);
		hBitmap = (HBITMAP)SelectObject(hdc, hBitmap);

		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(0xff, 0xff, 0xff));
		SetTextAlign(hdc, TA_TOP | TA_LEFT | TA_NOUPDATECP);

		BitBlt(hdc, 0, 0, cxcy, cxcy, hdc, 0, 0, BLACKNESS);

		for (const auto& [k, r] : out.glyphQuads)
		{
			ExtTextOut(hdc, int(floor(r.left)), int(floor(r.top)), 0, nullptr, &k, 1, nullptr);
		}

		// Unselect bitmap
		hBitmap = (HBITMAP)SelectObject(hdc, hBitmap);

		// Get bitmap info
		out.bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		if (GetDIBits(hdc, hBitmap, 0, 0, nullptr, &out.bitmapInfo, DIB_RGB_COLORS))
		{
			out.bitmapInfo.bmiHeader.biHeight = -abs(out.bitmapInfo.bmiHeader.biHeight);
			out.bitmapInfo.bmiHeader.biCompression = BI_RGB;
			out.DIBits.resize(out.bitmapInfo.bmiHeader.biSizeImage);
			GetDIBits(
				hdc,
				hBitmap,
				0,
				out.bitmapInfo.bmiHeader.biHeight,
				out.DIBits.data(),
				&out.bitmapInfo,
				DIB_RGB_COLORS
			);
			out.bitmapInfo.bmiHeader.biHeight *= -1;
		}

		SelectObject(hdc, hOldFont);
		DeleteDC(hdc);
		DeleteDC(hDisplayDc);
	}

	return out;
}
