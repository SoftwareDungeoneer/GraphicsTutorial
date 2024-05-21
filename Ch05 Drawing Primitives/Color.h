#ifndef GRAPHICS_TUTORIAL_COLOR_H
#define GRAPHICS_TUTORIAL_COLOR_H
#pragma once

#include <Windows.h>

struct ColorF;

struct Color {
	Color() {}
	Color(ColorF&);

	union {
		struct { char r, g, b, a; };
		DWORD color;
	};
};

struct ColorF {
	ColorF() {}
	ColorF(float _r, float _g, float _b, float _a = 1.0f):
		r(_r), g(_g), b(_b), a(_a) 
	{}
	ColorF(COLORREF color, unsigned char alpha = 255):
		ColorF(color, alpha / 255.f)
	{}
	ColorF(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a):
		ColorF(_r / 255.f, _g / 255.f, _b / 255.f, a / 255.f)
	{}

	ColorF(COLORREF color, float alpha)
	{
		r = GetRValue(color) / 255.f;
		g = GetGValue(color) / 255.f;
		b = GetBValue(color) / 255.f;
		a = alpha;
	}
	ColorF(Color c):
		ColorF((unsigned char)c.r, c.g, c.b, c.a)
	{}

	union {
		struct { float r, g, b, a; };
		float channels[4];
	};
};

inline Color::Color(ColorF& cf)
{
	r = static_cast<unsigned char>(cf.r * 255);
	g = static_cast<unsigned char>(cf.g * 255);
	b = static_cast<unsigned char>(cf.b * 255);
	a = static_cast<unsigned char>(cf.a * 255);
}
#endif // GRAPHICS_TUTORIAL_COLOR_H

