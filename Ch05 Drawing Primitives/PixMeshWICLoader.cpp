#include "PixMeshWICLoader.h"
#include "PixMeshUtil.h"

void PixMeshWICLoader::SetFactory(ComPtr<IWICImagingFactory> pFactory)
{
	m_pFactory = pFactory;
}

ComPtr<IWICImagingFactory> PixMeshWICLoader::GetFactory()
{
	if (!m_pFactory)
		CreateNewFactory();
	return m_pFactory;
}

ComPtr<IWICBitmapDecoder> PixMeshWICLoader::LoadImageFromUri(LPCTSTR lpszUri)
{
	if (!m_pFactory)
		CreateNewFactory();
	ComPtr<IWICBitmapDecoder> pDecoder;
	m_pFactory->CreateDecoderFromFilename(lpszUri, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, pDecoder);
	return pDecoder;
}

ComPtr<IWICFormatConverter> 
PixMeshWICLoader::GetConvertedFrame(ComPtr<IWICBitmapDecoder> pBitmap, UINT frame, GUID format)
{
	ComPtr<IWICFormatConverter> pOut {};
	if (!m_pFactory || !pBitmap)
		return pOut;

	UINT frameCount = 0;
	HRESULT hr = pBitmap->GetFrameCount(&frameCount);
	if (frame >= frameCount)
		frame = frameCount - 1;

	ComPtr<IWICBitmapFrameDecode> pFrame;
	if (SUCCEEDED(hr))
		hr = pBitmap->GetFrame(frame, pFrame);
	if (SUCCEEDED(hr))
		hr = m_pFactory->CreateFormatConverter(pOut);
	if (SUCCEEDED(hr))
		hr = pOut->Initialize(
			*pFrame,
			format,
			WICBitmapDitherTypeNone,
			NULL, 
			0.0,
			WICBitmapPaletteTypeMedianCut
		);
	return pOut;
}

std::vector<BYTE> PixMeshWICLoader::LoadImageDataFromUri(
	LPCTSTR lpszUri,
	D3D11_TEXTURE2D_DESC* desc)
{
	auto decoder = LoadImageFromUri(lpszUri);
	auto converter = GetConvertedFrame(decoder, 0, GUID_WICPixelFormat32bppRGBA);
	UINT cx, cy;
	converter->GetSize(&cx, &cy);
	UINT stride = 4 * cx;
	std::vector<BYTE> out(stride * cy);
	BYTE* pBuffer = out.data();
	converter->CopyPixels(NULL, stride, (UINT)out.size(), pBuffer);

	if (desc)
	{
		desc->Width = cx;
		desc->Height = cy;
		desc->MipLevels = 1;
		desc->ArraySize = 1;
		desc->Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc->SampleDesc.Count = 1;
		desc->SampleDesc.Quality = 0;
		desc->Usage = D3D11_USAGE_DEFAULT;
		desc->BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc->CPUAccessFlags = D3D11_CPU_ACCESS_FLAG(0);
		desc->MiscFlags = 0;
	}

	return out;
}

void PixMeshWICLoader::CreateNewFactory()
{
	m_pFactory = 0;
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		m_pFactory);
	phAssertCom(hr, "CoCreateInstance failed\n");
}
