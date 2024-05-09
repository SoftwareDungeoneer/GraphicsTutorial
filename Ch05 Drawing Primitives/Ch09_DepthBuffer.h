#ifndef GRAPHICS_TUTORIAL_CH09_DEPTH_BUFFER_H
#define GRAPHICS_TUTORIAL_CH09_DEPTH_BUFFER_H
#pragma once

#include "Renderer.h"

#include <d3d11.h>

#include <array>

#include "ComPtr.h"

class DepthBuffer : public Renderer
{
public:
	DepthBuffer(HWND h) : Renderer(h) {}

	virtual void Update(double elapsed);
	virtual void Render();

	struct Vertex
	{
		float Pos[3]{ 0, 0, 0 };
		float Color[4]{ 0, 0, 0, 1 };

		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};

protected:
	virtual void Initialize();

private:
	void CreateVertexBuffer();
	void CreateTextureRenderTarget();
	void CreateDepthStencilAndState();

	ComPtr<ID3D11VertexShader> depthVertexShader;
	ComPtr<ID3D11PixelShader> depthPixelShader;
	ComPtr<ID3D11InputLayout> inputLayout;

	ComPtr<ID3D11VertexShader> fullscreenVS;
	ComPtr<ID3D11PixelShader> fullscreenPS;
	ComPtr<ID3D11SamplerState> fullscreenSampler;

	ComPtr<ID3D11PixelShader> depthPS;

	ComPtr<ID3D11Buffer> vertexBuffer;

	ComPtr<ID3D11Texture2D> renderTexture;
	ComPtr<ID3D11RenderTargetView> renderTargetView;
	ComPtr<ID3D11ShaderResourceView> renderTextureSRV;

	ComPtr<ID3D11Texture2D> depthTexture;
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	ComPtr<ID3D11ShaderResourceView> depthStencilSRV;
	ComPtr<ID3D11DepthStencilState> depthStencilState;

	static const std::array<Vertex, 9> verts;
};

#endif // GRAPHICS_TUTORIAL_CH09_DEPTH_BUFFER_H