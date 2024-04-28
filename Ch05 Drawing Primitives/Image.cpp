#include "Image.h"

#include <map>
#include <utility>

namespace {
	struct FormatData {
		unsigned pixelByteWidth;
		GUID format;
	};

	std::map<Image::Format, FormatData> formatData = {
		std::make_pair<Image::Format, FormatData>(Image::Format::RGBA8, { 4, GUID_WICPixelFormat32bppRGBA }),
		std::make_pair<Image::Format, FormatData>(Image::Format::BGRA8, { 4, GUID_WICPixelFormat32bppBGRA }),
		std::make_pair<Image::Format, FormatData>(Image::Format::RGBA_FLOAT, { 16, GUID_WICPixelFormat128bppRGBAFloat }),
	};
}

HRESULT Image::Load(LPCTSTR filename)
{
	HRESULT hr{ S_OK };
	if (!pFactory)
		CreateNewFactory();
	
	if (FAILED(hr))
		return hr;

	pDecoder.Release();

	hr = pFactory->CreateDecoderFromFilename(
		filename,
		NULL, // Unknown vendor
		GENERIC_READ | GENERIC_WRITE, // Desired access
		WICDecodeMetadataCacheOnLoad,
		&*pDecoder
	);

	if (FAILED(hr))
		return hr;

	hr = pDecoder->GetFrameCount(&frameCount);
	selectedFrame = 0;

	if (FAILED(hr))
		return hr;

	hr = pDecoder->GetFrame(selectedFrame, &*pFrame);
	return hr;
}

HRESULT Image::CreateNewFactory()
{
	pFactory = nullptr;
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		(VOID**)&*pFactory);
	return hr;
}

unsigned Image::Width()
{
	[[maybe_unused]] unsigned width;
	[[maybe_unused]] unsigned height;
	if (pFrame)
		pFrame->GetSize(&width, &height);
	return width;
}

unsigned Image::Height()
{
	[[maybe_unused]] unsigned width;
	[[maybe_unused]] unsigned height;
	if (pFrame)
		pFrame->GetSize(&width, &height);
	return height;
}

unsigned Image::FrameCount()
{
	unsigned count{ 0 };
	if (pDecoder)
		pDecoder->GetFrameCount(&count);
	return count;
}

HRESULT Image::SelectFrame(unsigned frame)
{
	HRESULT hr{ S_OK };
	if (frame == selectedFrame && pFrame)
		return hr;

	selectedFrame = frame;
	if (pFrame)
		pFrame.Release();
	if (pDecoder)
		hr = pDecoder->GetFrame(selectedFrame, &*pFrame);
	return hr;
}

std::vector<BYTE> Image::GetFrameData(unsigned frame, Image::Format fmt)
{
	HRESULT hr{ SelectFrame(frame) };
	if (FAILED(hr))
		return std::vector<BYTE>();
	
	ComPtr<IWICFormatConverter> pConverter;
	hr = pFactory->CreateFormatConverter(&*pConverter);
	if (FAILED(hr))
		return std::vector<BYTE>();

	hr = pConverter->Initialize(
		*pFrame,
		formatData[fmt].format,
		WICBitmapDitherTypeNone,
		NULL,
		0.0,
		WICBitmapPaletteTypeMedianCut
	);

	if (FAILED(hr))
		return std::vector<BYTE>();
	
	UINT cx, cy;
	pConverter->GetSize(&cx, &cy);
	UINT stride = cx * formatData[fmt].pixelByteWidth;
	
	std::vector<BYTE> out(stride * cy);
	pConverter->CopyPixels(NULL, stride, static_cast<UINT>(out.size()), out.data());

	return out;
}

HRESULT Image::ConvertFrame(BYTE* pOut, UINT bufferSize, unsigned frame, Format fmt)
{
	HRESULT hr{ E_FAIL };
	if (!pOut)
		return hr;

	hr = SelectFrame(frame);
	
	ComPtr<IWICFormatConverter> pConverter;
	if (SUCCEEDED(hr))
		hr = pFactory->CreateFormatConverter(&*pConverter);

	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			*pFrame,
			formatData[fmt].format,
			WICBitmapDitherTypeNone,
			NULL,
			0.0,
			WICBitmapPaletteTypeMedianCut
		);
	}

	UINT cx, cy, stride;
	if (SUCCEEDED(hr))
	{
		pConverter->GetSize(&cx, &cy);
		stride = cx * formatData[fmt].pixelByteWidth;
		hr = pConverter->CopyPixels(NULL, stride, bufferSize, pOut);
	}

	return hr;
}
