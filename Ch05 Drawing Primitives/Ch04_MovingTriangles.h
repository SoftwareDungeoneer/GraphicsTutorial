#ifndef GRAPHICS_TUTORIAL_CH04_MOVING_TRIANGLES_H
#define GRAPHICS_TUTORIAL_CH04_MOVING_TRIANGLES_H
#pragma once

#include "Renderer.h"

class MovingTriangles : public Renderer
{
public:
	MovingTriangles(HWND h) : Renderer(h) {}

	virtual void Update(double elapsed);
	virtual void Render();

protected:
	virtual void Initialize();
};

#endif // GRAPHICS_TUTORIAL_CH04_MOVING_TRIANGLES_H
