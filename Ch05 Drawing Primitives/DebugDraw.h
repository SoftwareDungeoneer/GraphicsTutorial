#ifndef GRAPHICS_TUTORIAL_DEBUG_DRAW_H
#define GRAPHICS_TUTORIAL_DEBUG_DRAW_H
#pragma oncee

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>

#include <string>

#include "ComPtr.h"

class DebugDraw
{
public:
	DebugDraw(
		ComPtr<ID3D11Device> _device,
		ComPtr<ID3D11DeviceContext> _context,
		ComPtr<IDXGISwapChain> _swapChain
	);

	void DrawLineSS(POINT start, POINT end);
	void DrawTextSS(POINT topLeft, const std::string& text);

	void RenderFrameTimes(POINT topLeft);

private:
	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pContext;
	ComPtr<IDXGISwapChain> pSwapChain;

	ComPtr<ID3D11Texture2D> pFrameTimesTexture;
	ComPtr<ID3D11ShaderResourceView> pFrameTimesSRV;
	ComPtr<ID3D11RenderTargetView> pFrameTimesRTV;

	float frameTimes[256];
	unsigned frameHead{ 0 };
};

#endif // GRAPHICS_TUTORIAL_DEBUG_DRAW_H
