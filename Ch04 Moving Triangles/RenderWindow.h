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

	void Update(double elapsed);

private:
	RenderWindow(RenderWindow&&) = delete;
	RenderWindow(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;
	RenderWindow& operator=(const RenderWindow&) = delete;

	void RegisterWindowClass();
	HRESULT CreateUIWindow();
	void CreateD3DDevice();
	void InitializeRendering();
	void InitializeTriangles();
	D3D11_VIEWPORT ViewportFromTexture(ComPtr<ID3D11Texture2D>& tex);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM, LPARAM);
	LRESULT OnCreate();
	LRESULT OnDestroy();
	LRESULT OnSize();

	HWND hWnd{ 0 };

	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pDeviceContext;
	ComPtr<IDXGISwapChain> pSwapChain;

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Buffer> indexBuffer;
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> viewportCBuffer;
	ComPtr<ID3D11Buffer> instanceCBuffer;

	double elapsedInterval{ 0.0 };
	bool enableUpdate{ false };

	static LPCTSTR kWindowClassName;

	struct __declspec(align(16)) ViewportConstantBuffer {
		float viewportWidth;
		float viewportHeight;
	} vscbViewport;

	struct __declspec(align(16)) InstnaceConstantBuffer {
		float position[2];
		float color[4];
	} vscbInstanceBuffer;

	static const unsigned kNumTriangles = 10;
	struct TriangleData {
		Vec2 position;
		Vec2 veloicty;
		float color[4];
	} triangles[kNumTriangles];
};

#endif // RENDER_WINDOW_H
