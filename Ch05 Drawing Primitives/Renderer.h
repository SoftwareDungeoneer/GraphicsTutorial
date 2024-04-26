#ifndef GRAPHICS_TUTORIAL_RENDERER_H
#define GRAPHICS_TUTORIAL_RENDERER_H
#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include <string>

#include "ComPtr.h"

class Renderer
{
public:
	Renderer(HWND);

	void Resize(unsigned cx, unsigned cy);

	virtual std::string GetDescription() = 0;
	virtual void Update(double elapsed) = 0;
	virtual void Render() = 0;

protected:
	virtual void Initialize() = 0;

	D3D11_VIEWPORT ViewportFromTexture(ComPtr<ID3D11Texture2D>& tex);

	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pDeviceContext;
	ComPtr<IDXGISwapChain> pSwapChain;

	bool enableUpdate{ false };

	unsigned windowWidth;
	unsigned windowHeight;

private:
	void CreateD3DDevice();

	HWND hWnd;
};

#endif // GRAPHICS_TUTORIAL_RENDERER_H
