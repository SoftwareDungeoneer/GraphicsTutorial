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

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11Buffer> viewportCBuffer;
	ComPtr<ID3D11Buffer> instanceCBuffer;

	double elapsedSum{ 0.f };
	unsigned bgIndex{ 0 };

	struct __declspec(align(16)) InstanceCData
	{
		Vec2 pos;
		float radius;
		unsigned numVerts;
		float color[4];
	} instanceData;
};

#endif // CH05_DRAWING_PRIMITIVES_H
