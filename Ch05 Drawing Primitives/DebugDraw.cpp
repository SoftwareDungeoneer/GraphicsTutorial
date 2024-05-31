#include "DebugDraw.h"

#include <tchar.h>

#include "util.h"

constexpr LPCTSTR debugDrawFontFace = _T("Calibri");
constexpr unsigned debugFontSize = 19;

const ColorF DebugDraw::defaultDebugColor{ 1.0f, 1.0f, 1.0f, 1.0f };

inline POINTF PointToPointF(POINT p)
{
	return { p.x * 1.0f, p.y * 1.0f };
}

DebugDraw::DebugDraw(
	ComPtr<ID3D11Device> _device,
	ComPtr<ID3D11DeviceContext> _context,
	ComPtr<IDXGISwapChain> _swapChain
) :
	pDevice(_device),
	pContext(_context),
	pSwapChain(_swapChain),
	debugFont(_device, _context)
{
	static_assert(offsetof(DebugText, color) == 16);

	InitializeCriticalSection(&m_lock);

	debugFont.LoadFont(debugDrawFontFace, debugFontSize);
	memset(frameTimes, 0.0f, sizeof(frameTimes));
	CreateTextures();
}

DebugDraw::~DebugDraw()
{
	DeleteCriticalSection(&m_lock);
}

void DebugDraw::CreateTextures()
{
	if (!pDevice)
		return;
}

void DebugDraw::PushFrameTime(float time)
{
	EnterCriticalSection(&m_lock);
	frameTimes[frameHead++] = time;
	frameHead %= countof(frameTimes);
	LeaveCriticalSection(&m_lock);
}

void DebugDraw::DrawLineSS(POINT start, POINT end, ColorF color)
{
	lineSegments.emplace_back(PointToPointF(start), PointToPointF(end), color);
}

void DebugDraw::DrawTextSS(POINT topLeft, const std::string& text, ColorF color)
{
	textSections.emplace_back(PointToPointF(topLeft), color, text);
}

void DebugDraw::RenderFrameTimes(POINT topLeft)
{
	fpsLocation = PointToPointF(topLeft);
}

void DebugDraw::Render()
{
	RenderLines();
	RenderText();
	RenderFPS();
}

void DebugDraw::RenderLines()
{
}

void DebugDraw::RenderText()
{
	for (const auto& text : textSections)
	{
		std::wstring str{ WideStringFromString(text.text) };
		debugFont.RenderString(text.topLeft, str.c_str(), (unsigned)str.size(), text.color);
	}
}

void DebugDraw::RenderFPS()
{
}
