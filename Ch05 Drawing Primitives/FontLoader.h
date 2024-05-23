#ifndef GRAPHICS_TUTORIAL_WINDOWS_FONT_LOADER_H
#define GRAPHICS_TUTORIAL_WINDOWS_FONT_LOADER_H
#pragma once

#include <Windows.h>
#include <map>
#include <vector>

#include "util.h"

struct FontData
{
	TEXTMETRIC tm{ 0 };
	BITMAPINFO bitmapInfo{ 0 };
	std::vector<BYTE> DIBits;
	std::map<TCHAR, RECTF> glyphQuads;
};

namespace FontLoader
{
	FontData LoadFont(LPCTSTR facename, int height);

	FontData ConvertFontToAtlas(HFONT hFont);
}

#endif //GRAPHICS_TUTORIAL_WINDOWS_FONT_LOADER_H
