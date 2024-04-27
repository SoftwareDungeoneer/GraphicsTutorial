#ifndef GRAPHICS_TUTORIAL_IMAGE_H
#define GRAPHICS_TUTORIAL_IMAGE_H
#pragma once

#include <Windows.h>
#include <vector>

class Image
{
public:
	enum class Formats {
		RGBA8,
		BGRA8,
		RGBA_FLOAT
	};

	Image(
		unsigned cx,
		unsigned cy,
		Formats fmt = Formats::RGBA8,
		unsigned dataLen = 0,
		char* initialData = nullptr
	);

	unsigned Width() { return width; }
	unsigned Height() {	return height; }

	static Image LoadFromFile(LPCTSTR filename, Formats fmt = Formats::RGBA8);

private:
	std::vector<char> data;

	unsigned width;
	unsigned height;
};

#endif // GRAPHICS_TUTORIAL_IMAGE_H
