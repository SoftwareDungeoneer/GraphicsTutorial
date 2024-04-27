#include "Ch07_Textures.h"

#include <tchar.h>

#include <array>

#include "util.h"

const D3D11_INPUT_ELEMENT_DESC Textures::Vertex::desc[]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

void Textures::Initialize()
{
	auto vsBytes = LoadFile(_T("TexturedQuad.vsc"));
	auto psBytes = LoadFile(_T("TexturedQuad.psc"));
	pDevice->CreateVertexShader(vsBytes.data(), vsBytes.size(), nullptr, &*vertexShader);
	pDevice->CreatePixelShader(psBytes.data(), psBytes.size(), nullptr, &*pixelShader);

	pDevice->CreateInputLayout(
		Textures::Vertex::desc,
		countof(Textures::Vertex::desc),
		vsBytes.data(),
		vsBytes.size(),
		&*inputLayout
	);

	enableUpdate = true;
}

void Textures::Update(double elapsed)
{
	if (!enableUpdate)
		Initialize();
}

void Textures::Render()
{
	if (!enableUpdate)
		return;

	constexpr float bgBlack[]{ 0.f, 0.f, 0.f, 1.f };
	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, bgBlack);

	D3D11_VIEWPORT viewport{ ViewportFromTexture(pBackBuffer) };

	std::array<ID3D11Buffer*, 1> vertexBuffers{
		*vertexBuffer,
	};
	unsigned strides[]{ 0 };
	unsigned offsets[]{ 0 };
	pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers.data(), strides, offsets);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->IASetInputLayout(*inputLayout);

	pSwapChain->Present(1, 0);
}
