#ifndef GRAPHICS_TUTORIAL_DEBUG_DRAW_H
#define GRAPHICS_TUTORIAL_DEBUG_DRAW_H
#pragma once

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>

#include <string>
#include <utility>
#include <vector>

#include "Color.h"
#include "RenderFont.h"

#include "ComPtr.h"

class DebugDraw
{
public:
	DebugDraw(
		ComPtr<ID3D11Device> _device,
		ComPtr<ID3D11DeviceContext> _context,
		ComPtr<IDXGISwapChain> _swapChain
	);
	~DebugDraw();

	void DrawLineSS(POINT start, POINT end, ColorF color = defaultDebugColor);
	void DrawTextSS(POINT topLeft, const std::string& text, ColorF color = defaultDebugColor);

	void PushFrameTime(float time);
	void RenderFrameTimes(POINT topLeft);

	void Render();

private:
	struct LineSegment {
		POINTF begin, end;
		ColorF color;
	};
	struct DebugText {
		POINTF topLeft;
		__declspec(align(16)) ColorF color;
		std::string text;
	};

	void CreateTextures();

	void RenderLines();
	void RenderText();
	void RenderFPS();

	CRITICAL_SECTION m_lock;

	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pContext;
	ComPtr<IDXGISwapChain> pSwapChain;
	
	ComPtr<ID3D11Texture2D> pFontAtlas;
	ComPtr<ID3D11ShaderResourceView> pFontAtlasSRV;

	ComPtr<ID3D11Texture2D> pFrameTimesTexture;
	ComPtr<ID3D11Texture2D> pFrameTimeFontAtlass;
	ComPtr<ID3D11ShaderResourceView> pFrameTimesSRV;
	ComPtr<ID3D11ShaderResourceView> pFrameTimeFontSRV;
	ComPtr<ID3D11RenderTargetView> pFrameTimesRTV;

	std::vector<LineSegment> lineSegments;
	std::vector<DebugText> textSections;

	RenderFont debugFont;

	float frameTimes[256];
	unsigned frameHead{ 0 };
	POINTF fpsLocation{ -1.f, -1.f };

	static const ColorF defaultDebugColor;
};

#endif // GRAPHICS_TUTORIAL_DEBUG_DRAW_H
