#ifndef PIX_MESH_WIC_LOADER_H
#define PIX_MESH_WIC_LOADER_H
#pragma once

#include <Wincodec.h>
#pragma comment(lib, "Windowscodecs.lib")

#include <d3d11.h>

#include <vector>
#include "ComPtr.h"

class PixMeshWICLoader
{
public:
	PixMeshWICLoader() {}
	PixMeshWICLoader(ComPtr<IWICImagingFactory> pFactory):m_pFactory{pFactory} {}
	PixMeshWICLoader(const PixMeshWICLoader&) = default;
	PixMeshWICLoader(PixMeshWICLoader&&) = default;
	~PixMeshWICLoader() = default;

	void SetFactory(ComPtr<IWICImagingFactory> pFactory);
	ComPtr<IWICImagingFactory> GetFactory();

	ComPtr<IWICBitmapDecoder> LoadImageFromUri(LPCTSTR lpszUri);

	ComPtr<IWICFormatConverter>
	GetConvertedFrame(
		ComPtr<IWICBitmapDecoder> pDecoder,
		UINT idx, 
		GUID format
	);

	std::vector<BYTE> LoadImageDataFromUri(
		LPCTSTR lpszUri,
		D3D11_TEXTURE2D_DESC* descOut = 0);

protected:
	void CreateNewFactory();

	ComPtr<IWICImagingFactory> m_pFactory;
};

#endif // PIX_MESH_WIC_LOADER_H
