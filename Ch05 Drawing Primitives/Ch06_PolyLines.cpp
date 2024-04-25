#include "Ch06_PolyLines.h"

#include <tchar.h>

#include <array>

#include "util.h"

using Vertex = PolyLines::Vertex;
const D3D11_INPUT_ELEMENT_DESC Vertex::desc[] = {
	{ "BEGINPOS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 4 },
	{ "ENDPOS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 4 },
};

struct Line {
	Vertex begin;
	Vertex end;
} const kLines[] = {
	{ { 50, 50 }, { 80, 50 } },
	{ { 65, 35 }, { 65, 75 } },
	{ { 10, 400 }, { 700, 400 } },
};

struct LineParams {
	Vertex begin;
	Vertex end;
	enum : unsigned {
		kFalloffRationalFalloff,
		kFalloffRationalFunction
	} falloffMode;
	float falloffMax;
	float scale;
	float clip;
	float rationalPowers[2];
	float reserved0[2];
	float color[3];
	float reserved1;
} lineSets[countof(kLines)];

constexpr unsigned kLineParamsCBufferSize = aligned_size_16<LineParams>;

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
		aligned_size_16<Line>,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_CONSTANT_BUFFER,
		0, 0, 0
	};
	D3D11_SUBRESOURCE_DATA srd{ kLines, 0, 0 };
	pDevice->CreateBuffer(&bufferDesc, &srd, &*lineVertexConstants);

	bufferDesc.ByteWidth = aligned_size_16<D3D11_VIEWPORT>;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pDevice->CreateBuffer(&bufferDesc, nullptr, &*viewportCBuffer);


	bufferDesc.ByteWidth = kLineParamsCBufferSize;
	pDevice->CreateBuffer(&bufferDesc, nullptr, &*lineParamsBuffer);

	// Initialize lineSets params
	float colors[countof(kLines)][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 }
	};

	for (unsigned n = 0; n < countof(kLines); ++n)
	{
		lineSets[n].begin = kLines[n].begin;
		lineSets[n].end = kLines[n].end;
		lineSets[n].falloffMode = LineParams::kFalloffRationalFunction;
		lineSets[n].falloffMax = 5;
		lineSets[n].scale = 1.2f;
		lineSets[n].clip = 0;
		lineSets[n].rationalPowers[0] = 1;
		lineSets[n].rationalPowers[1] = 2;
		lineSets[n].reserved0[0] = lineSets[n].reserved0[1] = 0.f;
		lineSets[n].reserved1 = 0;
		for (unsigned j = 0; n < 3; ++n)
			lineSets[n].color[j] = colors[n][j];
	}

	D3D11_BLEND_DESC blendDesc;
	ZeroInitialize(blendDesc);
	auto& rtBlend = blendDesc.RenderTarget[0];
	rtBlend.BlendEnable = TRUE;
	rtBlend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtBlend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtBlend.BlendOp = D3D11_BLEND_OP_ADD;
	rtBlend.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtBlend.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtBlend.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtBlend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	pDevice->CreateBlendState(&blendDesc, &*pBlendState);

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

	//unsigned strides[] = { 0 };
	//unsigned offsets[] = { 0 };
	//ID3D11Buffer* vertBuffers[] = { *linesVertexBuffer };
	//pDeviceContext->IASetVertexBuffers(0, 1, vertBuffers, strides, offsets);
	pDeviceContext->IASetInputLayout(*polyLineInputLayout);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->RSSetViewports(1, &viewport);
	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);
	pDeviceContext->OMSetBlendState(*pBlendState, nullptr, 0xFFFFFFFF);

	std::array<ID3D11Buffer*, 2> vsBuffers = {
		*viewportCBuffer,
		*lineVertexConstants
	};
	pDeviceContext->VSSetConstantBuffers(0, vsBuffers.size(), vsBuffers.data());
	pDeviceContext->VSSetShader(*polyLineVS, nullptr, 0);

	pDeviceContext->PSSetConstantBuffers(0, 1, &*lineParamsBuffer);
	pDeviceContext->PSSetShader(*polyLinePS, nullptr, 0);
	
	for (unsigned n = 0; n < countof(kLines); ++n)
	{
		pDeviceContext->UpdateSubresource(*lineVertexConstants, 0, nullptr, &kLines[n], 0, 0);
		pDeviceContext->UpdateSubresource(*lineParamsBuffer, 0, nullptr, &lineSets[n], 0, 0);
		pDeviceContext->Draw(4, 0);
	}

	pSwapChain->Present(1, 0);
}
