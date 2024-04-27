#ifndef GRAPHICS_TUTORIAL_CH07_TEXTURES_H
#define GRAPHICS_TUTORIAL_CH07_TEXTURES_H

#include "Renderer.h"

#include <d3d11.h>

#include "ComPtr.h"

class Textures : public Renderer
{
public:
	Textures(HWND h) : Renderer(h) {}

	virtual void Update(double elapsed);
	virtual void Render();

protected:
	virtual void Initialize();

private:
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;

	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Texture2D> texture;

public:
	struct Vertex
	{
		float Pos2D[2]{ 0, 0 };
		float Texcoord[2]{ 0, 0 };

		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};
};

#endif // GRAPHICS_TUTORIAL_CH07_TEXTURES_H
