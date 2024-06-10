#include "DebugDraw.h"

#include <tchar.h>

#include "util.h"

constexpr LPCTSTR debugDrawFontFace = _T("Calibri");
constexpr unsigned debugFontSize = 19;

const ColorF DebugDraw::defaultDebugColor{ 1.0f, 1.0f, 1.0f, 1.0f };

const D3D11_INPUT_ELEMENT_DESC DebugDraw::LineSegment::desc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

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

void DebugDraw::ResizeLinesVertexBuffer()
{
	D3D11_BUFFER_DESC desc;
	ZeroInitialize(desc);
	desc.ByteWidth = sizeof(LineSegment) * linesHighWaterMark;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	pLinesVertexBuffer.Release();
	pDevice->CreateBuffer(&desc, nullptr, &*pLinesVertexBuffer);
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
	lineSegments.emplace_back(PointToPointF(start), color);
	lineSegments.emplace_back(PointToPointF(end), color);
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
	if (!lineSegments.size())
		return;

	if (lineSegments.size() > linesHighWaterMark)
		ResizeLinesVertexBuffer();

	pContext->UpdateSubresource(*pLinesVertexBuffer, 0, nullptr, lineSegments.data(), 0, 0);

	unsigned strides[] = { sizeof(LineSegment) };
	unsigned offsets[] = { 0 };

	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	pContext->IASetInputLayout(*linesInputLayout);
	pContext->IASetVertexBuffers(0, 1, &*pLinesVertexBuffer, strides, offsets);

	pContext->Draw(lineSegments.size(), 0);
	
	lineSegments.clear(); // Maintains capacity, https://en.cppreference.com/w/cpp/container/vector/clear
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
