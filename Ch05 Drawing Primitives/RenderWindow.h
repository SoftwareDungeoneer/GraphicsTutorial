#ifndef RENDER_WINDOW_H
#define RENDER_WINDOW_H
#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "ComPtr.h"

#include "Vec2.h"

class RenderWindow
{
public:
	RenderWindow();
	~RenderWindow();

	HRESULT Create();

	virtual void Update(double elapsed) = 0;
	virtual void Render() = 0;

protected:
	HWND hWnd{ 0 };
	unsigned windowWidth{ 0 };
	unsigned windowHeight{ 0 };

	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pDeviceContext;
	ComPtr<IDXGISwapChain> pSwapChain;

	D3D11_VIEWPORT ViewportFromTexture(ComPtr<ID3D11Texture2D>& tex);

private:
	RenderWindow(RenderWindow&&) = delete;
	RenderWindow(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;
	RenderWindow& operator=(const RenderWindow&) = delete;

	void RegisterWindowClass();
	HRESULT CreateUIWindow();
	void CreateD3DDevice();

	virtual void Initialize() = 0;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM, LPARAM);
	LRESULT OnCreate();
	LRESULT OnDestroy();
	LRESULT OnSize();

	static LPCTSTR kWindowClassName;
};

#endif // RENDER_WINDOW_H
