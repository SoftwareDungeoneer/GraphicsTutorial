#ifndef GRAPHICS_TUTORIAL_CH07_FONT_ATLAS_H
#define GRAPHICS_TUTORIAL_CH07_FONT_ATLAS_H

#include "Renderer.h"

#include <d3d11.h>

#include <memory>

#include "ComPtr.h"
#include "Image.h"
#include "FontLoader.h"

class FontAtlas : public Renderer
{
public:
	FontAtlas(HWND h) : Renderer(h) {}

	virtual void Update(double elapsed);
	virtual void Render();

protected:
	virtual void Initialize();

private:
	FontData fontData;
	ComPtr<ID3D11Texture2D> fontAtlas;
	ComPtr<ID3D11ShaderResourceView> fontAtlasSRV;

	std::unique_ptr<Image> starImage;

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;

	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> viewportConstantBuffer;
	ComPtr<ID3D11Texture2D> texture;
	ComPtr<ID3D11ShaderResourceView> textureSRV;
	ComPtr<ID3D11SamplerState> samplerState;

	void RemapFontBits();

public:
	struct Vertex
	{
		float Pos2D[2]{ 0, 0 };
		float Texcoord[2]{ 0, 0 };

		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};
};

#endif // GRAPHICS_TUTORIAL_CH07_FONT_ATLAS_H
