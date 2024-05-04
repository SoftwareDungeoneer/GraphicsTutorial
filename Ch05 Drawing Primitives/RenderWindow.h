#ifndef RENDER_WINDOW_H
#define RENDER_WINDOW_H
#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include <memory>
#include <vector>

#include "DebugDataWindow.h"
#include "ToolWindow.h"
#include "Renderer.h"

class RenderWindow
{
public:
	RenderWindow();
	~RenderWindow();

	HRESULT Create();

	void Update(double interval);

	enum class Demos : unsigned {
		FirstTriangle,
		MovingTriangles,
		DrawingPrimitives,
		PolyLines,
		Textures,
		FontAtlas,

		COUNT
	};

	void SetDemo(Demos);

protected:
	HWND hWnd{ 0 };
	unsigned windowWidth{ 0 };
	unsigned windowHeight{ 0 };

	std::shared_ptr<Renderer> activeRenderer;
	std::shared_ptr<ToolWindow> toolWindow{ nullptr };
	std::shared_ptr<DebugDataWindow> debugDataWindow{ nullptr };

private:
	std::shared_ptr<DebugDataWindow::DataBlock> debugDataStore{ nullptr };

	RenderWindow(RenderWindow&&) = delete;
	RenderWindow(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;
	RenderWindow& operator=(const RenderWindow&) = delete;

	void RegisterWindowClass();
	HRESULT CreateUIWindow();

	void UpdateDebugInfo(const std::string& key, const std::string& value);
	void UpdateDebugPosition();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM, LPARAM);
	LRESULT OnCreate();
	LRESULT OnDestroy();
	LRESULT OnMove();
	LRESULT OnSize();
	LRESULT OnNotifyDebugDataDestroyed();

	static LPCTSTR kWindowClassName;
};

#endif // RENDER_WINDOW_H
