#ifndef GRAPHICS_TUTORIAL_CH07_TEXTURES_H
#define GRAPHICS_TUTORIAL_CH07_TEXTURES_H

#include "Renderer.h"

class Textures : public Renderer
{
public:
	Textures(HWND h) : Renderer(h) {}

	virtual void Update(double elapsed);
	virtual void Render();

protected:
	virtual void Initialize();
};

#endif // GRAPHICS_TUTORIAL_CH07_TEXTURES_H
