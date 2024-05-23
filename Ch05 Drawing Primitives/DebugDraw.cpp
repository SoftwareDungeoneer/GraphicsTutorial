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
	pSwapChain(_swapChain)
{
	static_assert(offsetof(DebugText, color) == 16);

	InitializeCriticalSection(&m_lock);

	debugFont = FontLoader::LoadFont(debugDrawFontFace, debugFontSize);
	CreateTextures();
}

DebugDraw::~DebugDraw()
{
	DeleteCriticalSection(&m_lock);
}

std::vector<BYTE> DebugDraw::RemapFontBits(FontData& fontData)
{
	std::vector<BYTE> vb;
	constexpr float coefficients[4] = { 0.114f, 0.587f, 0.299f, 0.f };
	auto length = fontData.DIBits.size() / 4;
	vb.reserve(length);
	for (unsigned pos{ 0 }; pos < length; ++pos)
	{
		float acc{ 0.f };
		unsigned x = (pos) % fontData.bitmapInfo.bmiHeader.biWidth;
		unsigned y = (pos) / fontData.bitmapInfo.bmiHeader.biWidth;
		auto bp = fontData.DIBits.data() + (4 * pos);
		for (unsigned n{ 0 }; n < 4; ++n)
		{
			acc += coefficients[n] * (*(bp + n));
		}
		vb.emplace_back(BYTE(acc));
	}
	return vb;
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

void Render()
{

}