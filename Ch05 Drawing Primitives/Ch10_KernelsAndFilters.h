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

protected:
	virtual void Initialize();

private:
	ComPtr<ID3D11VertexShader> quadVertexShader;
	ComPtr<ID3D11InputLayout> quadInputLayout;
	ComPtr<ID3D11Buffer> quadInputCBuffer;

	ComPtr<ID3D11PixelShader> unfilteredShader;
	ComPtr<ID3D11PixelShader> kernelShader;
	ComPtr<ID3D11Buffer> kernelShaderCBuffer;

	ComPtr<ID3D11SamplerState> linearSampler; // Used to sample texture inputs
	
	std::vector<ComPtr<ID3D11Texture2D>> inputTextures;

	static const Vertex unfilteredVerts[];
	static const Vertex filteredVerts[];

	std::vector<ComPtr<ID3D11Texture2D>> images;

	void LoadShaders();
	void LoadTextures();
	void CreateConstantBuffers();
	void CreateSamplers();
	void CreateVertexBuffers();
};

#endif // GRAPHICS_TUTORIAL_CH10_KERNELS_AND_FILTERS_H
