#include "Ch05_DrawingPrimitives.h"

#include <tchar.h>

#include <array>
#include <numbers>

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


using LineNode = DrawingPrimitives::LineNode;
const D3D11_INPUT_ELEMENT_DESC LineNode::desc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(LineNode, color), D3D11_INPUT_PER_VERTEX_DATA, 0}
};

DrawingPrimitives::LineNode DrawingPrimitives::linePoints[] = {
	{ {  20,  20 }, 1 },
	{ { 750,  40 }, 2 },
	{ { 750, 500 }, 3 },
	{ { 20, 599 }, 4 },
	{ {  20,  20 }, 1 },
};



void DrawingPrimitives::Initialize() {
	ZeroInitialize(instanceData);

	auto clientVsBuffer = LoadFile(_T("ClientToNdc.cso"));
	auto vsBuffer = LoadFile(_T("RegularPolyVS.cso"));
	auto psBuffer = LoadFile(_T("SolidFill.cso"));
	pDevice->CreateVertexShader(clientVsBuffer.data(), clientVsBuffer.size(), nullptr, &*clientToNdcVS);
	pDevice->CreateVertexShader(vsBuffer.data(), vsBuffer.size(), nullptr, &*primitivesVS);
	pDevice->CreatePixelShader(psBuffer.data(), psBuffer.size(), nullptr, &*pixelShader);

	D3D11_BUFFER_DESC bufferDesc{
		aligned_size_16<D3D11_VIEWPORT>,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_CONSTANT_BUFFER,
		0,
		0,
		0
	};
	pDevice->CreateBuffer(&bufferDesc, nullptr, &*viewportCBuffer);

	instanceData.numVerts = 3;
	instanceData.radius = 50.f;
	instanceData.angle = 0.f;

	bufferDesc.ByteWidth = aligned_size_16<InstanceCData>;
	pDevice->CreateBuffer(&bufferDesc, nullptr, &*instanceCBuffer);

	for (auto& pt : linePoints)
	{
		unsigned n = unsigned(pt.color[0]);
		memcpy(pt.color, kBackgroundColors[n], sizeof(float[4]));
	}
	bufferDesc.ByteWidth = sizeof(linePoints); // aligned_size_16<LineNode>* countof(linePoints);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA srd
	{
		linePoints,
		0,
		0,
	};
	pDevice->CreateBuffer(&bufferDesc, &srd, &*linesVertexBuffer);
	pDevice->CreateInputLayout(LineNode::desc, countof(LineNode::desc), clientVsBuffer.data(), clientVsBuffer.size(), &*linesInputLayout);
	enableUpdate = true;

}

void DrawingPrimitives::Update(double elapsed) {
	if (!enableUpdate)
		Initialize();

	//instanceData.angle += .5f * elapsed;
	
	static unsigned counter{ 0 };
	elapsedSum += elapsed;
	if (elapsedSum > 0.2)
	{
		++counter;
		if (counter == 3)
		{
			++bgIndex;
			counter = 0;
			instanceData.numVerts += 1;
			if (instanceData.numVerts > 36)
				instanceData.numVerts = 3;
		}

		elapsedSum = 0.0;
		bgIndex %= countof(kBackgroundColors) - 1;
	}
	instanceData.pos.x = windowWidth * 3 / 4.f;
	instanceData.pos.y = windowHeight / 2.f;
	instanceData.radius = 125.f;
	if (instanceData.angle >= 2.f * std::numbers::pi_v<float>)
		instanceData.angle -= 2.f * std::numbers::pi_v<float>;
	instanceData.mode = 0;
	memcpy(instanceData.color, kBackgroundColors[bgIndex + 1], sizeof(float[4]));
}

void DrawingPrimitives::Render() {
	if (!enableUpdate)
		return;

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
	unsigned strides[] = { sizeof(LineNode) };
	unsigned offsets[] = { 0 };
	
	ID3D11Buffer* pNullptr{ nullptr };
	
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->IASetVertexBuffers(0, 1, &pNullptr, strides, offsets);
	pDeviceContext->RSSetViewports(1, &viewport);
	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);

	pDeviceContext->VSSetConstantBuffers(0, vsBuffers.size(), vsBuffers.data());
	pDeviceContext->VSSetShader(*primitivesVS, nullptr, 0);
	pDeviceContext->PSSetShader(*pixelShader, nullptr, 0);

	pDeviceContext->Draw(2 * instanceData.numVerts + 1, 0);

	instanceData.pos.x = windowWidth * 1.f / 4.f;
	instanceData.mode = 1;
	pDeviceContext->UpdateSubresource(*instanceCBuffer, 0, nullptr, &instanceData, 0, 0);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	pDeviceContext->Draw(instanceData.numVerts + 1, 0);
	//pDeviceContext->Draw(4, 0);

	// Draw lines

	pDeviceContext->VSSetShader(*clientToNdcVS, nullptr, 0);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	pDeviceContext->IASetInputLayout(*linesInputLayout);
	pDeviceContext->IASetVertexBuffers(0, 1, &*linesVertexBuffer, strides, offsets);

	pDeviceContext->Draw(countof(linePoints), 0);

	pSwapChain->Present(1, 0);
}

