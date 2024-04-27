#include "Ch04_MovingTriangles.h"

#include <tchar.h>

#include <array>
#include <random>

#include "util.h"

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

namespace {
	struct Vertex {
		float pos[2];
		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};
	const D3D11_INPUT_ELEMENT_DESC Vertex::desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		// Per instance data
		{ "IPOS", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "ICOLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1  }
	};
}

void MovingTriangles::Initialize()
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
		{ -8, 0 },
		{  8, 0 },
		{  0, 12 }
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

	D3D11_BUFFER_DESC cbufferDesc{
		sizeof(vscbViewport),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_CONSTANT_BUFFER,
		0,
		0, // No Misc flags
		0, // Not a structured buffer
	};
	pDevice->CreateBuffer(&cbufferDesc, nullptr, &*viewportCBuffer);

	cbufferDesc.ByteWidth = sizeof(triangles);
	cbufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	pDevice->CreateBuffer(&cbufferDesc, nullptr, &*instanceCBuffer);

	std::random_device rd;
	std::mt19937 engine(rd());
	std::uniform_real_distribution<float> dist;

	ZeroInitialize(triangles);
	for (auto& tri : triangles)
	{
		tri.position.x = dist(engine) * 750 + 25;
		tri.position.y = dist(engine) * 550;
		tri.velocity.x = 120 * (2 * dist(engine) - 1);
		tri.velocity.y = 120 * (2 * dist(engine) - 1);
		tri.color[0] = dist(engine);
		tri.color[1] = dist(engine);
		tri.color[2] = dist(engine);
		tri.color[3] = 1.f;
	}

	enableUpdate = true;
}

void MovingTriangles::Update(double elapsed)
{
	if (!enableUpdate)
		Initialize();

	for (auto& tri : triangles)
	{
		tri.position += tri.velocity * float(elapsed);
		if (tri.position.x <= 8.f || tri.position.x >= 792.f)
			tri.velocity.x *= -1;
		if (tri.position.y <= 0.f || tri.position.y >= 588.f)
			tri.velocity.y *= -1;
	}
}

void MovingTriangles::Render()
{
	static unsigned bgIndex{ 0 };

	if (!enableUpdate)
		return;

	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, kBackgroundColors[0]);

	// Get back buffer description and build viewport
	D3D11_VIEWPORT viewport{ ViewportFromTexture(pBackBuffer) };

	// Update vertex shader constants
	vscbViewport.viewportWidth = viewport.Width;
	vscbViewport.viewportHeight = viewport.Height;
	pDeviceContext->UpdateSubresource(*viewportCBuffer, 0, nullptr, &vscbViewport, 0, 0);

	pDeviceContext->UpdateSubresource(*instanceCBuffer, 0, nullptr, &triangles, 0, 0);

	// Set up input assembler
	unsigned strides[2] = { sizeof(Vertex), sizeof(TriangleData) };
	unsigned offsets[2] = { 0, 0 };
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->IASetIndexBuffer(*indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	pDeviceContext->IASetInputLayout(*inputLayout);
	std::array<ID3D11Buffer*, 2> vsIndexBuffers = {
		*vertexBuffer,
		*instanceCBuffer
	};
	pDeviceContext->IASetVertexBuffers(0, 2, vsIndexBuffers.data(), strides, offsets);

	// Set up vertex shadr
	std::array<ID3D11Buffer*, 1> vsConstantBuffers = {
		*viewportCBuffer,
	};
	pDeviceContext->VSSetShader(*vertexShader, nullptr, 0);
	pDeviceContext->VSSetConstantBuffers(0, 1, vsConstantBuffers.data());

	// Set up rasterizer
	pDeviceContext->RSSetViewports(1, &viewport);

	// Set up Output Merger
	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);

	// Set up pixel shader
	pDeviceContext->PSSetShader(*pixelShader, nullptr, 0);

	// Dispatch draw
	pDeviceContext->DrawIndexedInstanced(3, kNumTriangles, 0, 0, 0);

	pSwapChain->Present(1, 0);
}

