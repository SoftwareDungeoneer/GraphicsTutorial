#ifndef GRAPHICS_TUTORIAL_WINDOWS_FONT_LOADER_H
#define GRAPHICS_TUTORIAL_WINDOWS_FONT_LOADER_H
#pragma once

#include <Windows.h>
#include <map>
#include <vector>

struct FontData
{
	TEXTMETRIC tm;
	BITMAPINFO bitmapInfo;
	std::vector<BYTE> DIBits;
	std::map<TCHAR, RECT> glyphQuads;
};

namespace FontLoader
{
	FontData LoadFont(LPCTSTR facename, int height);
	FontData ConvertFontToAtlas(HFONT hFont);
}

#endif //GRAPHICS_TUTORIAL_WINDOWS_FONT_LOADER_H
