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
		float Texcoord[2]{ 0, 0 };

		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};

protected:
	virtual void Initialize();

private:
	void CreateTextureRenderTarget();

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11InputLayout> inputLayout;

	ComPtr<ID3D11VertexShader> fullscreenVS;
	ComPtr<ID3D11PixelShader> fullscreenPS;
	ComPtr<ID3D11SamplerState> fullscreenSampler;

	ComPtr<ID3D11Texture2D> renderTexture;
	ComPtr<ID3D11RenderTargetView> renderTargetView;
	ComPtr<ID3D11ShaderResourceView> renderTextureSRV;

	ComPtr<ID3D11Texture2D> depthTexture;
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	ComPtr<ID3D11DepthStencilState> depthStencilState;

	std::array<Vertex, 9> verts;
};

#endif // GRAPHICS_TUTORIAL_CH09_DEPTH_BUFFER_H