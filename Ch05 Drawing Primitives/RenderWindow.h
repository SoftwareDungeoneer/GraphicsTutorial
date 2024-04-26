#ifndef RENDER_WINDOW_H
#define RENDER_WINDOW_H
#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include <memory>
#include <vector>

#include "ToolWindow.h"
#include "Renderer.h"

class RenderWindow
{
public:
	RenderWindow();
	~RenderWindow();

	HRESULT Create();

	void Update(double interval);

protected:
	HWND hWnd{ 0 };
	unsigned windowWidth{ 0 };
	unsigned windowHeight{ 0 };

	std::shared_ptr<Renderer> activeRenderer;

private:
	RenderWindow(RenderWindow&&) = delete;
	RenderWindow(const RenderWindow&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;
	RenderWindow& operator=(const RenderWindow&) = delete;

	void RegisterWindowClass();
	HRESULT CreateUIWindow();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM, LPARAM);
	LRESULT OnCreate();
	LRESULT OnDestroy();
	LRESULT OnSize();

	static LPCTSTR kWindowClassName;
};

#endif // RENDER_WINDOW_H
