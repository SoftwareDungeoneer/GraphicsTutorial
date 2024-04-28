#ifndef GRAPHICS_TUTORIAL_IMAGE_H
#define GRAPHICS_TUTORIAL_IMAGE_H
#pragma once

#include <Windows.h>
#include <Wincodec.h>
#pragma comment(lib, "Windowscodecs.lib")

#include "ComPtr.h"

#include <vector>

class Image
{
public:
	enum class Format {
		RGBA8,
		BGRA8,
		RGBA_FLOAT,

		COUNT
	};

	HRESULT Load(LPCTSTR filename);

	unsigned Width();
	unsigned Height();
	unsigned FrameCount();
	HRESULT SelectFrame(unsigned frameIdx);

	HRESULT ConvertFrame(BYTE* pOut, UINT bufferSize, unsigned frame, Format fmt = Format::RGBA8);
	std::vector<BYTE> GetFrameData(unsigned frame, Format fmt = Format::RGBA8);


private:
	ComPtr<IWICImagingFactory> pFactory;
	ComPtr<IWICBitmapDecoder> pDecoder;
	ComPtr<IWICBitmapFrameDecode> pFrame;

	unsigned frameCount{ 0 };
	unsigned selectedFrame{ 0 };

	HRESULT CreateNewFactory();
};

#endif // GRAPHICS_TUTORIAL_IMAGE_H
