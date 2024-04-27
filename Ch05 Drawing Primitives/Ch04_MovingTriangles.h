#ifndef GRAPHICS_TUTORIAL_CH04_MOVING_TRIANGLES_H
#define GRAPHICS_TUTORIAL_CH04_MOVING_TRIANGLES_H
#pragma once

#include "Renderer.h"

#include "Vec2.h"

class MovingTriangles : public Renderer
{
public:
	MovingTriangles(HWND h) : Renderer(h) {}

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
	ComPtr<ID3D11Buffer> viewportCBuffer;
	ComPtr<ID3D11Buffer> instanceCBuffer;

	struct __declspec(align(16)) ViewportConstantBuffer {
		float viewportWidth;
		float viewportHeight;
	} vscbViewport;

	struct __declspec(align(16)) InstnaceConstantBuffer {
		float position[2];
		float color[4];
	} vscbInstanceBuffer;

	static const unsigned kNumTriangles = 100;
	struct TriangleData {
		Vec2 position;
		Vec2 velocity;
		float color[4];
	} triangles[kNumTriangles];
};

#endif // GRAPHICS_TUTORIAL_CH04_MOVING_TRIANGLES_H
