#include "Ch06_PolyLines.h"

#include <tchar.h>

#include <array>

#include "util.h"

using Vertex = PolyLines::Vertex;
const D3D11_INPUT_ELEMENT_DESC Vertex::desc[] = {
	{ "POSITION0", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 6 },
};

struct Line {
	Vertex begin;
	Vertex end;
} const kLines[] = {
	{ { 50, 50 }, { 50, 80 } },
	{ { 65, 35 }, { 65, 75 } },
	{ { 10, 400 }, { 700, 400 } },
};

void PolyLines::Initialize()
{
	auto polyLineVsBuffer = LoadFile(_T("PolyLineVS.cso"));
	auto polyLinePsBuffer = LoadFile(_T("PolyLinePS.cso"));
	pDevice->CreateVertexShader(polyLineVsBuffer.data(), polyLineVsBuffer.size(), nullptr, &*polyLineVS);
	pDevice->CreatePixelShader(polyLinePsBuffer.data(), polyLinePsBuffer.size(), nullptr, &*polyLinePS);
	
	pDevice->CreateInputLayout(
		Vertex::desc,
		countof(Vertex::desc),
		polyLineVsBuffer.data(),
		polyLineVsBuffer.size(),
		&*polyLineInputLayout
	);

	D3D11_BUFFER_DESC bufferDesc{
		sizeof(kLines),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0, 0, 0
	};
	D3D11_SUBRESOURCE_DATA srd{ kLines, 0, 0 };
	pDevice->CreateBuffer(&bufferDesc, &srd, &*linesVertexBuffer);

	bufferDesc.ByteWidth = aligned_size_16<D3D11_VIEWPORT>;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pDevice->CreateBuffer(&bufferDesc, nullptr, &*viewportCBuffer);

	enableUpdate = true;
}

void PolyLines::Update(double elapsed)
{

}

void PolyLines::Render()
{
	if (!enableUpdate)
		return;

	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	float clearColor[4] = { 0.f, 0.f, 0.f, 1.f };
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, clearColor );

	D3D11_VIEWPORT viewport{ ViewportFromTexture(pBackBuffer) };
	pDeviceContext->UpdateSubresource(*viewportCBuffer, 0, nullptr, &viewport, 0, 0);

	unsigned strides[] = { 0 };
	unsigned offsets[] = { 0 };
	ID3D11Buffer* vertBuffers[] = { *linesVertexBuffer };
	pDeviceContext->IASetVertexBuffers(0, 1, vertBuffers, strides, offsets);
	pDeviceContext->IASetInputLayout(*polyLineInputLayout);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->RSSetViewports(1, &viewport);
	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);

	std::array<ID3D11Buffer*, 1> vsBuffers = {
		*viewportCBuffer
	};
	pDeviceContext->VSSetConstantBuffers(0, vsBuffers.size(), vsBuffers.data());
	pDeviceContext->VSSetShader(*polyLineVS, nullptr, 0);
	pDeviceContext->PSSetShader(*polyLinePS, nullptr, 0);


}
