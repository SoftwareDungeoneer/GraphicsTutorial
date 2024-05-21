#include "DebugDraw.h"

#include "Color.h"
#include "util.h"

DebugDraw::DebugDraw(
	ComPtr<ID3D11Device> _device,
	ComPtr<ID3D11DeviceContext> _context,
	ComPtr<IDXGISwapChain> _swapChain
) :
	pDevice(_device),
	pContext(_context),
	pSwapChain(_swapChain)
{
	CreateTextures();
}

void DebugDraw::CreateTextures()
{
	if (!pDevice)
		return;
}

void DebugDraw::DrawLineSS(POINT start, POINT end)
{
	lineSegments.push_back(std::make_pair(start, end));
}

void DebugDraw::DrawTextSS(POINT topLeft, const std::string& text)
{
}

void DebugDraw::RenderFrameTimes(POINT topLeft)
{
}

void Render()
{

}