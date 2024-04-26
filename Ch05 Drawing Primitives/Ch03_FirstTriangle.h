#ifndef GRAPHICS_TUTORIAL_CH03_FIRST_TRIANGLE_H
#define GRAPHICS_TUTORIAL_CH03_FIRST_TRIANGLE_H
#pragma once

#include "Renderer.h"

class FirstTriangle : public Renderer
{
public:
	FirstTriangle(HWND h) : Renderer(h) {};

	virtual void Update(double elapsed);
	virtual void Render();

protected:
	virtual void Initialize();

private:
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Buffer> indexBuffer;
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> vsConstants;

	unsigned bgIndex{ 0 };
	double elapsedInterval{ 0.0 };

	struct __declspec(align(16)) VSConstantBuffer {
		float fillColor[4];
		float viewportWidth;
		float viewportHeight;
	} vscbuffer;
	static_assert(offsetof(VSConstantBuffer, viewportHeight) == 20);
};

#endif // GRAPHICS_TUTORIAL_CH03_FIRST_TRIANGLE_H
