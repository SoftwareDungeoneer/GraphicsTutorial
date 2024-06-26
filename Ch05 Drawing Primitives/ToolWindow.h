#ifndef GRAPHICS_TUTORIAL_TOOL_WINDOW
#define GRAPHICS_TUTORIAL_TOOL_WINDOW
#pragma once

#include <Windows.h>

#include <array>
#include <memory>

#include "Settings.h"

class RenderWindow;

class ToolWindow
{
public:
	ToolWindow(RenderWindow* rw, std::shared_ptr<Settings> _s);
	~ToolWindow();

	void Create();

protected:
	HWND hWnd{ 0 };
	HWND hDemoSelector{ 0 };

	HFONT uiFont{ nullptr };
	TEXTMETRIC uiFontMetrics;

private:
	ToolWindow(ToolWindow&&) = delete;
	ToolWindow(const ToolWindow&) = delete;
	ToolWindow& operator=(ToolWindow&&) = delete;
	ToolWindow& operator=(const ToolWindow&) = delete;

	void Register();
	void CreateUIWindow();

	enum class eLabels {
		DemoSelection,

		COUNT
	};
	std::array<unsigned, static_cast<size_t>(eLabels::COUNT)> labelPositions;
	static const std::array<LPCTSTR, static_cast<size_t>(eLabels::COUNT)> labelStrings;

	RenderWindow* pRenderWindow;

	std::shared_ptr<Settings> appSettings;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM, LPARAM);
	LRESULT OnCreate();
	LRESULT OnDestroy();
	LRESULT OnMove();
	LRESULT OnSize();
	LRESULT OnPaint();
	LRESULT OnCommand(WPARAM, LPARAM);

	LRESULT OnDemoSelector(unsigned command);

	static LPCTSTR kWindowClassName;
};

#endif // GRAPHICS_TUTORIAL_TOOL_WINDOW