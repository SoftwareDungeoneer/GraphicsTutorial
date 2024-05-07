#ifndef GRAPHICS_TUTORIAL_WINDOW_SETTINGS_H
#define GRAPHICS_TUTORIAL_WINDOW_SETTINGS_H
#pragma once

#include <Windows.h>

#include <memory>

struct Settings
{
	union {
		unsigned flags;
		struct {
			unsigned RememberWindowPosition : 1;
			unsigned RememberWindowSize : 1;
		};
	};

	POINT mainWindowPos;
	SIZE mainWindowSize;
	POINT toolWindowPos;
	SIZE toolWindowSize;
	POINT debugWindowPos;
	SIZE debugWindowSize;

	static Settings GetDefaultSettings();

	static int Serialize(LPCTSTR filename, const std::shared_ptr<Settings> pSettings);
	static int Deserialize(LPCTSTR filename, std::shared_ptr<Settings> pSettings);
};


#endif // GRAPHICS_TUTORIAL_WINDOW_SETTINGS_H
