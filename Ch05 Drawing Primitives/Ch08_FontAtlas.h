#ifndef GRAPHICS_TUTORIAL_CH07_FONT_ATLAS_H
#define GRAPHICS_TUTORIAL_CH07_FONT_ATLAS_H

#include "Renderer.h"

#include <d3d11.h>

#include <array>
#include <memory>
#include <vector>

#include "ComPtr.h"
#include "Image.h"
#include "FontLoader.h"

class FontAtlas : public Renderer
{
public:
	FontAtlas(HWND h) : Renderer(h) {}

	virtual void Update(double elapsed);
	virtual void Render();

	struct Vertex
	{
		float Pos2D[2]{ 0, 0 };
		float Texcoord[2]{ 0, 0 };

		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};
protected:
	virtual void Initialize();

private:
	FontData fontData;

	// Textures and samplers
	ComPtr<ID3D11Texture2D> fontAtlas;
	ComPtr<ID3D11Texture2D> fontAtlasGrayscale;
	ComPtr<ID3D11SamplerState> samplerState;

	// SRVs
	ComPtr<ID3D11ShaderResourceView> fontAtlasSRV;
	ComPtr<ID3D11ShaderResourceView> fontAtlasGrayscaleSRV;

	// Shaders
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11InputLayout> inputLayout;

	// Buffers
	ComPtr<ID3D11Buffer> fontAtlasVB;
	ComPtr<ID3D11Buffer> grayscaleVB;
	ComPtr<ID3D11Buffer> viewportConstantBuffer;
	ComPtr<ID3D11Buffer> colorConstantBuffer;

	void LoadShaders();
	void CreateFontTextures();
	void CreateVertexBuffers();
	void CreateConstantBuffers();

	void RescaleFontUVs();
	std::vector<BYTE> RemapFontBits();

	std::array<Vertex, 4> WorldRectToVertices(RECT r);
};

#endif // GRAPHICS_TUTORIAL_CH07_FONT_ATLAS_H
