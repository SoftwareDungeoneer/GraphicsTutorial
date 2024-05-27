#ifndef GRAPHICS_TUTORIAL_RENDER_FONT_H
#define GRAPHICS_TUTORIAL_RENDER_FONT_H
#pragma once

#include <dxgi.h>
#include <d3d11.h>

#include "Color.h"
#include "FontLoader.h"

#include "ComPtr.h"

class RenderFont
{
public:
	RenderFont(ComPtr<ID3D11Device> _device, ComPtr<ID3D11DeviceContext> _context);

	void LoadFont(LPCTSTR fontFace, int height);
	
	void RenderString(POINTF topleft, LPCTSTR lpsz, unsigned nChars, const ColorF color);

private:
	void LoadShaders();
	void CreateConstantBuffers();
	void CreateBlendState();
	void CreateSamplerState();

	void CreateFontTexture();
	std::vector<BYTE> RemapFontBits();
	void RescaleFontUVs();

	void AllocateBufferSpace(unsigned nChars);

	void LayoutString(POINTF topleft, LPCTSTR lpsz, unsigned nChars);


	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pContext;

	ComPtr<ID3D11BlendState> alphaEnableBlendState;
	ComPtr<ID3D11SamplerState> pointSampler;

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	
	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Buffer> viewportConstantBuffer;
	ComPtr<ID3D11Buffer> colorConstantBuffer;

	ComPtr<ID3D11Texture2D> fontTexture;
	ComPtr<ID3D11ShaderResourceView> fontSRV;

	ComPtr<ID3D11Buffer> indexBuffer;
	ComPtr<ID3D11Buffer> vertexBuffer;
	unsigned cchHighWater{ 0 };

	FontData fontData;
};

#endif // GRAPHICS_TUTORIAL_RENDER_FONT_H
