#ifndef GRAPHICS_TUTORIAL_CH10_KERNELS_AND_FILTERS_H
#define GRAPHICS_TUTORIAL_CH10_KERNELS_AND_FILTERS_H
#pragma once

#include "Renderer.h"

#include <d3d11.h>

#include <memory>
#include <vector>

#include "Image.h"

#include "ComPtr.h"

class KernelFilters : public Renderer
{
public:
	KernelFilters(HWND h) : Renderer(h) {}

	virtual void Update(double elapsed);
	virtual void Render();

	struct Vertex {
		float Pos[2]{ 0, 0 };
		float Tex[2]{ 0 , 0 };

		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};

	virtual void NotifyKeyUp(unsigned key);

protected:
	virtual void Initialize();
	virtual void ResizeNotify();

private:
	enum KernelSel {
		kBoxBlur,
		kGauss3,
		kGauss5,
		kGauss7,
		kSobelX,
		kSobelY,

		COUNT
	};

	ComPtr<ID3D11VertexShader> quadVertexShader;
	ComPtr<ID3D11InputLayout> quadInputLayout;
	ComPtr<ID3D11Buffer> quadInputCBuffer;

	ComPtr<ID3D11PixelShader> unfilteredShader;
	ComPtr<ID3D11PixelShader> kernelShader;
	ComPtr<ID3D11Buffer> kernelShaderCBuffer;

	ComPtr<ID3D11SamplerState> linearSampler; // Used to sample texture inputs
	
	std::vector<std::pair<ComPtr<ID3D11Texture2D>, ComPtr<ID3D11ShaderResourceView>>> inputTextures;
	decltype(inputTextures)::iterator selectedTexture;

	ComPtr<ID3D11Buffer> unfilteredVertBuffer;
	ComPtr<ID3D11Buffer> outputVertBuffer;
	
	Vertex unfilteredVerts[4];
	Vertex filteredVerts[4];

	ComPtr<ID3D11Texture2D> kernelTexture;
	ComPtr<ID3D11ShaderResourceView> kernelSrv;
	KernelSel activeKernel{ kBoxBlur };

	void LoadShaders();
	void LoadTextures();
	void CreateConstantBuffers();
	void CreateSamplers();
	void CreateVertexBuffers();

	void UpdateKernelTexture();
};

#endif // GRAPHICS_TUTORIAL_CH10_KERNELS_AND_FILTERS_H
