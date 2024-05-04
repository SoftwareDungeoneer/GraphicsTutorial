#ifndef GRAPHICS_TUTORIAL_DEBUG_DATA_WINDOW_H
#define GRAPHICS_TUTORIAL_DEBUG_DATA_WINDOW_H
#pragma once

#include <Windows.h>

#include <map>
#include <memory>
#include <string>

class DebugDataWindow
{
private:
	struct SectionParams
	{
		union
		{
			unsigned flags;
			struct {
				unsigned expanded : 1;
			};
		};
	};

public:
	using DataBlock = std::map<std::string, std::pair<std::map<std::string, std::string>, SectionParams>>;

	DebugDataWindow(HWND rw, std::shared_ptr<DataBlock> _db);
	~DebugDataWindow();

	HRESULT Create();

	void UpdateWindow();

	void UpdateValue(
		const std::string& section,
		const std::string& key,
		const std::string& value
	);

	void RemoveSection(const std::string& section);
	void RemoveValue(const std::string& section, const std::string& key);

private:
	HWND hWnd{ 0 };
	HWND renderWindow{ 0 };
	std::shared_ptr<DataBlock> valueMap;

	DebugDataWindow(DebugDataWindow&&) = delete;
	DebugDataWindow(const DebugDataWindow&) = delete;
	DebugDataWindow& operator=(DebugDataWindow&&) = delete;
	DebugDataWindow& operator=(const DebugDataWindow&) = delete;

	void RegisterWindowClass();
	HRESULT CreateUIWindow();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM, LPARAM);
	LRESULT OnCreate();
	LRESULT OnDestroy();
	LRESULT OnPaint();

	static LPCTSTR kWindowClassName;
};

#endif // GRAPHICS_TUTORIAL_DEBUG_DATA_WINDOW_H
