#include "Ch05_DrawingPrimitives.h"

#include <tchar.h>

#include <array>

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

void DrawingPrimitives::Initialize() {
	ZeroInitialize(instanceData);

	auto vsBuffer = LoadFile(_T("RegularPolyVS.cso"));
	auto psBuffer = LoadFile(_T("SolidFill.cso"));
	pDevice->CreateVertexShader(vsBuffer.data(), vsBuffer.size(), nullptr, &*vertexShader);
	pDevice->CreatePixelShader(psBuffer.data(), psBuffer.size(), nullptr, &*pixelShader);

	instanceData.numVerts = 3;
	instanceData.radius = 50.f;
}

void DrawingPrimitives::Update(double elapsed) {
	elapsedSum += elapsed;
	if (elapsedSum > 1.0)
	{
		++bgIndex;
//		instanceData.numVerts += 1;
		if (instanceData.numVerts > 32)
			instanceData.numVerts = 3;
		elapsedSum = 0.0;
		bgIndex %= countof(kBackgroundColors) - 1;
		instanceData.pos.x = windowWidth / 2.f;
		instanceData.pos.y = windowHeight / 2.f;
		instanceData.radius = 50.f;
		memcpy(instanceData.color, kBackgroundColors[bgIndex], sizeof(float[4]));
	}
}

void DrawingPrimitives::Render() {
	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, kBackgroundColors[0]);

	D3D11_VIEWPORT viewport{ ViewportFromTexture(pBackBuffer) };
	pDeviceContext->UpdateSubresource(*viewportCBuffer, 0, nullptr, &viewport, 0, 0);
	pDeviceContext->UpdateSubresource(*instanceCBuffer, 0, nullptr, &instanceData, 0, 0);

	std::array<ID3D11Buffer*, 2> vsBuffers = {
		*viewportCBuffer,
		*instanceCBuffer
	};

	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->VSSetConstantBuffers(0, vsBuffers.size(), vsBuffers.data());
	pDeviceContext->VSSetShader(*vertexShader, nullptr, 0);
	pDeviceContext->PSSetShader(*pixelShader, nullptr, 0);

	pDeviceContext->Draw(instanceData.numVerts, 0);

	pSwapChain->Present(1, 0);
}

