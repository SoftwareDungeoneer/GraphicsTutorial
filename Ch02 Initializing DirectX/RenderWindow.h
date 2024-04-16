#ifndef RENDER_WINDOW_H
#define RENDER_WINDOW_H
#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "ComPtr.h"

class RenderWindow
{
public:
	RenderWindow();
	~RenderWindow();

	HRESULT Create();

	void Update(double elapsed);

private:
	RenderWindow(RenderWindow&&) = delete;
	RenderWindow(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;
	RenderWindow& operator=(const RenderWindow&) = delete;

	void RegisterWindowClass();
	HRESULT CreateUIWindow();
	void CreateD3DDevice();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM, LPARAM);
	LRESULT OnCreate();
	LRESULT OnDestroy();

	HWND hWnd{ 0 };

	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pDeviceContext;
	ComPtr<IDXGISwapChain> pSwapChain;

	double elapsedInterval{ 0.0 };
	bool enableUpdate{ false };

	static LPCTSTR kWindowClassName;
};

#endif // RENDER_WINDOW_H
