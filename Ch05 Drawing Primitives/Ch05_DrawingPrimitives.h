#ifndef CH05_DRAWING_PRIMITIVES_H
#define CH05_DRAWING_PRIMITIVES_H
#pragma once

#include "RenderWindow.h"

#include <d3d11.h>
#include "ComPtr.h"

class DrawingPrimitives : public RenderWindow
{
	virtual void Update(double elapsed);
	virtual void Render();

private:
	virtual void Initialize();

	ComPtr<ID3D11VertexShader> primitivesVS;
	ComPtr<ID3D11VertexShader> clientToNdcVS;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11Buffer> viewportCBuffer;
	ComPtr<ID3D11Buffer> instanceCBuffer;
	ComPtr<ID3D11Buffer> linesVertexBuffer;
	ComPtr<ID3D11InputLayout> linesInputLayout;

	double elapsedSum{ 0.f };
	unsigned bgIndex{ 0 };

	struct __declspec(align(16)) InstanceCData
	{
		Vec2 pos;
		float radius;
		unsigned numVerts;
		float color[4];
	} instanceData;

public:
	struct __declspec(align(16)) LineNode {
		float pos[2];
		float color[4];
		static const D3D11_INPUT_ELEMENT_DESC desc[];
	} static linePoints[];
};

#endif // CH05_DRAWING_PRIMITIVES_H
