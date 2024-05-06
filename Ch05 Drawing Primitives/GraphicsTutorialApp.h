#ifndef GRAPHICS_TUTORIAL_APP_H
#define GRAPHICS_TUTORIAL_APP_H
#pragma once

#include <memory>

#include "RenderWindow.h"
#include "Settings.h"

class GraphicsTutorialApp
{
public:
	int Run();

private:
	std::shared_ptr<RenderWindow> mainWindow;

	Settings appSettings;

	double qptFrequency{ 0 };
	double qptStartup{ 0 };
	LARGE_INTEGER timeLastupdate{ .QuadPart = 0 };
};
#endif // GRAPHICS_TUTORIAL_APP_H
