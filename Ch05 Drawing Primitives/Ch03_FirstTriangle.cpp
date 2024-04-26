#include "Ch03_FirstTriangle.h"

#include <tchar.h>

#include <map>
#include <vector>

#include "util.h"

namespace
{
	struct Vertex {
		float pos[2];
		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};
	const D3D11_INPUT_ELEMENT_DESC Vertex::desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

#define RGBA_FLT(r, g, b) { r, g, b, 1.0f }
	constexpr float kBackgroundColors[][4]{
		RGBA_FLT(0.f, 0.f, 0.f),
		RGBA_FLT(1.f, 0.f, 0.f),
		RGBA_FLT(1.f, 1.f, 0.f),
		RGBA_FLT(0.f, 1.f, 0.f),
		RGBA_FLT(0.f, 1.f, 1.f),
		RGBA_FLT(0.f, 0.f, 1.f),
		RGBA_FLT(1.f, 0.f, 1.f),
		RGBA_FLT(1.f, 1.f, 1.f),
	};
}

void FirstTriangle::Initialize()
{
	auto vsBytes = LoadFile(_T("ClientToNdc.cso"));
	auto psBytes = LoadFile(_T("SolidFill.cso"));

	pDevice->CreateVertexShader(vsBytes.data(), vsBytes.size(), nullptr, &*vertexShader);
	pDevice->CreatePixelShader(psBytes.data(), psBytes.size(), nullptr, &*pixelShader);

	constexpr unsigned short indices[] = { 0, 2, 1 };
	constexpr D3D11_BUFFER_DESC indexBufferDesc{
		sizeof(indices),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER,
		0, // No CPU access
		0, // Mo misc flags
		0, // Not a structured buffer
	};
	const D3D11_SUBRESOURCE_DATA indexBufferSrd{ &indices, 0, 0 };
	pDevice->CreateBuffer(&indexBufferDesc, &indexBufferSrd, &*indexBuffer);

	const Vertex verts[] = {
		{ 400, 127 },
		{ 200, 473 },
		{ 600, 473 }
	};
	pDevice->CreateInputLayout(Vertex::desc, countof(Vertex::desc), vsBytes.data(), vsBytes.size(), &*inputLayout);

	constexpr D3D11_BUFFER_DESC vertexBufferDesc{
		sizeof(verts),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0, // No CPU access
		0, // No Misc flags
		0, // Not a structured buffer
	};
	const D3D11_SUBRESOURCE_DATA vertexBufferSrd{ &verts, 0, 0 };
	pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferSrd, &*vertexBuffer);

	constexpr D3D11_BUFFER_DESC cbufferDesc{
		sizeof(vscbuffer),
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_CONSTANT_BUFFER,
		D3D11_CPU_ACCESS_WRITE,
		0, // No Misc flags
		0, // Not a structured buffer
	};
	pDevice->CreateBuffer(&cbufferDesc, nullptr, &*vsConstants);
}

void FirstTriangle::Update(double elapsed)
{
	if (!enableUpdate)
		return;

	elapsedInterval += elapsed;
	if (elapsedInterval < 0.2)
		return;

	++bgIndex;
	bgIndex %= countof(kBackgroundColors) - 1;

	elapsedInterval = 0.0;
}

void FirstTriangle::Render()
{
	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, kBackgroundColors[countof(kBackgroundColors) - 1]);

	UINT nViewports{ 1 };
	D3D11_VIEWPORT viewport{ ViewportFromTexture(pBackBuffer) };

	// Update vertex shader constants
	vscbuffer.viewportWidth = viewport.Width;
	vscbuffer.viewportHeight = viewport.Height;
	memcpy(vscbuffer.fillColor, kBackgroundColors[bgIndex], sizeof(vscbuffer.fillColor));

	pDeviceContext->UpdateSubresource(*vsConstants, 0, nullptr, &vscbuffer, 0, 0);

	// Set up input assembler
	unsigned stride = sizeof(Vertex);
	unsigned offset = 0;
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->IASetIndexBuffer(*indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	pDeviceContext->IASetInputLayout(*inputLayout);
	pDeviceContext->IASetVertexBuffers(0, 1, &*vertexBuffer, &stride, &offset);

	// Set up vertex shadr
	pDeviceContext->VSSetShader(*vertexShader, nullptr, 0);
	pDeviceContext->VSSetConstantBuffers(0, 1, &*vsConstants);

	// Set up rasterizer
	pDeviceContext->RSSetViewports(1, &viewport);

	// Set up Output Merger
	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);

	// Set up pixel shader
	pDeviceContext->PSSetShader(*pixelShader, nullptr, 0);

	// Dispatch draw
	pDeviceContext->DrawIndexed(3, 0, 0);

	pSwapChain->Present(1, 0);
}
