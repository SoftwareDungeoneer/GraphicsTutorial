#ifndef GRAPHICS_TUTORIAL_CH11_PING_PONG_BUFFERS_H
#define GRAPHICS_TUTORIAL_CH11_PING_PONG_BUFFERS_H
#pragma once

#include "Renderer.h"

#include <d3d11.h>

#include <memory>
#include <vector>

#include "Image.h"

#include "ComPtr.h"

class PingPongBuffers : public Renderer
{
public:
	PingPongBuffers(HWND h) : Renderer(h) {}

	virtual void Update(double elapsed);
	virtual void Render();

	struct Vertex {
		float Pos[2]{ 0, 0 };
		float Tex[2]{ 0 , 0 };

		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};

protected:
	virtual void Initialize();
	virtual void ResizeNotify();

private:
	ComPtr<ID3D11VertexShader> quadVertexShader;
	ComPtr<ID3D11InputLayout> quadInputLayout;
	ComPtr<ID3D11Buffer> quadInputCBuffer;

	ComPtr<ID3D11PixelShader> unfilteredShader;
	ComPtr<ID3D11PixelShader> kernelShader;
	ComPtr<ID3D11Buffer> kernelShaderCBuffer;

	ComPtr<ID3D11SamplerState> linearSampler; // Used to sample texture inputs

	ComPtr<ID3D11Texture2D> inputTexture;
	ComPtr<ID3D11ShaderResourceView> inputSRV;

	ComPtr<ID3D11Buffer> unfilteredVertBuffer;
	ComPtr<ID3D11Buffer> outputVertBuffer;
	ComPtr<ID3D11Buffer> thirdPassVertBuffer;
	ComPtr<ID3D11Buffer> seventhPassVertBuffer;

	Vertex unfilteredVerts[4];
	Vertex filteredVerts[4];

	ComPtr<ID3D11Texture2D> kernelTexture;
	ComPtr<ID3D11ShaderResourceView> kernelSrv;

	struct TextureBuffer
	{
		ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11ShaderResourceView> srv;
		ComPtr<ID3D11RenderTargetView> rtv;
	} pingPongBuffers[2];
	unsigned inputBuffers{ 0 };

	void LoadShaders();
	void LoadTextures();
	void CreateConstantBuffers();
	void CreateSamplers();
	void CreateVertexBuffers();
	void CreatePingPongBuffers();

	void UpdateKernelTexture();

	void BlurPass(ComPtr<ID3D11ShaderResourceView> input);
};

#endif // GRAPHICS_TUTORIAL_CH11_PING_PONG_BUFFERS_H
