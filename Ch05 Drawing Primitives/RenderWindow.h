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
#include "Settings.h"

class RenderWindow
{
public:
	RenderWindow(std::shared_ptr<Settings>);
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
		DepthBuffer,
		Kernels,
		PingPongBuffers,

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
	std::shared_ptr<Settings> appSettings;
	std::shared_ptr<DebugDataWindow::DataBlock> debugDataStore{ nullptr };

	RenderWindow(RenderWindow&&) = delete;
	RenderWindow(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;
	RenderWindow& operator=(const RenderWindow&) = delete;

	void RegisterWindowClass();
	HRESULT CreateUIWindow();

	void UpdateDebugInfo(const std::string& key, const std::string& value);
	void UpdatePosition();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM, LPARAM);
	LRESULT OnCreate();
	LRESULT OnDestroy();
	LRESULT OnCommand(WPARAM, LPARAM);
	LRESULT OnMove();
	LRESULT OnSize();
	LRESULT OnKeyDown(WPARAM);
	LRESULT OnKeyUp(WPARAM);
	LRESULT OnChar(WPARAM);
	LRESULT OnNotifyDebugDataDestroyed();

	// Command handlers
	LRESULT OnFileExit();
	LRESULT OnViewSettings();
	static LPCTSTR kWindowClassName;
};

#endif // RENDER_WINDOW_H
