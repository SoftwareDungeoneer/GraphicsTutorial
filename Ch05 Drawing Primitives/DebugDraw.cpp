#include "DebugDraw.h"

#include <tchar.h>

#include "util.h"

constexpr LPCTSTR debugDrawFontFace = _T("Calibri");
constexpr unsigned debugFontSize = 19;

const ColorF DebugDraw::defaultDebugColor{ 0.0f, 1.0f, 0.0f, 1.0f };

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

	auto clientVsBuffer = LoadFile(_T("ClientToNdc.cso"));
	auto solidFillPSBuffer = LoadFile(_T("SolidFill.cso"));
	pDevice->CreateVertexShader(clientVsBuffer.data(), clientVsBuffer.size(), nullptr, &*clientSpaceVertexShader);
	pDevice->CreatePixelShader(solidFillPSBuffer.data(), solidFillPSBuffer.size(), nullptr, &*solidFillPixelShader);

	pDevice->CreateInputLayout(
		DebugDraw::LineSegment::desc,
		countof(DebugDraw::LineSegment::desc),
		clientVsBuffer.data(),
		clientVsBuffer.size(),
		&*linesInputLayout
	);

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

	EnterCriticalSection(&m_lock);

	pLinesVertexBuffer.Release();
	pDevice->CreateBuffer(&desc, nullptr, &*pLinesVertexBuffer);

	LeaveCriticalSection(&m_lock);
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
	EnterCriticalSection(&m_lock);
	lineSegments.emplace_back(PointToPointF(start), color);
	lineSegments.emplace_back(PointToPointF(end), color);
	LeaveCriticalSection(&m_lock);
}

void DebugDraw::DrawTextSS(POINT topLeft, const std::string& text, ColorF color)
{
	EnterCriticalSection(&m_lock);
	textSections.emplace_back(PointToPointF(topLeft), color, text);
	LeaveCriticalSection(&m_lock);
}

void DebugDraw::RenderFrameTimes(POINT topLeft)
{
	EnterCriticalSection(&m_lock);
	fpsLocation = PointToPointF(topLeft);
	LeaveCriticalSection(&m_lock);
}

void DebugDraw::Render()
{
	EnterCriticalSection(&m_lock);
	
	pContext->VSSetShader(*clientSpaceVertexShader, nullptr, 0);
	pContext->PSSetShader(*solidFillPixelShader, nullptr, 0);

	RenderLines();
	RenderText();
	RenderFPS();

	LeaveCriticalSection(&m_lock);
}

void DebugDraw::RenderLines()
{
	if (!lineSegments.size())
		return;

	if (lineSegments.size() > linesHighWaterMark)
	{
		linesHighWaterMark = (unsigned)lineSegments.size();
		ResizeLinesVertexBuffer();
	}

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
	textSections.clear();
}

void DebugDraw::RenderFPS()
{
}
