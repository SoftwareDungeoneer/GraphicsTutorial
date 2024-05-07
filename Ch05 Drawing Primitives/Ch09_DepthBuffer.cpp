#include "Ch09_DepthBuffer.h"

#include <tchar.h>

#include "util.h"

void DepthBuffer::Initialize()
{
	auto vsBytes = LoadFile(_T("Ch09_Depth2D.vsc"));
	auto psBytes = LoadFile(_T("CH09_Depth2D.psc"));
	auto fullscreenVsBytes = LoadFile(_T("FullscreenQuad.vsc"));
	auto fullscreenPsBytes = LoadFile(_T("TexturedQuad.psc"));

	pDevice->CreateVertexShader(vsBytes.data(), vsBytes.size(), nullptr, &*vertexShader);
	pDevice->CreateVertexShader(fullscreenVsBytes.data(), fullscreenVsBytes.size(), nullptr, &*fullscreenVS);
	pDevice->CreatePixelShader(psBytes.data(), psBytes.size(), nullptr, &*pixelShader);
	pDevice->CreatePixelShader(fullscreenPsBytes.data(), fullscreenPsBytes.size(), nullptr, &*fullscreenPS);

	CreateTextureRenderTarget();

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroInitialize(samplerDesc);
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.f;
	pDevice->CreateSamplerState(&samplerDesc, &*fullscreenSampler);
}

void DepthBuffer::CreateTextureRenderTarget()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.Height = 768;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc = { 1, 0 };
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	pDevice->CreateTexture2D(&texDesc, nullptr, &*renderTexture);

	// Create the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroInitialize(srvDesc);
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D = { 0, 1 };
	pDevice->CreateShaderResourceView(*renderTexture, &srvDesc, &*renderTextureSRV);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroInitialize(rtvDesc);
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	pDevice->CreateRenderTargetView(*renderTexture, &rtvDesc, &*renderTargetView);
}

void DepthBuffer::Update(double elapsed)
{
	if (!enableUpdate)
		Initialize();
	enableUpdate = true;
}

void DepthBuffer::Render()
{
	pDeviceContext->ClearState();

	constexpr float clearColor[4]{ 1.f, 0.f, 1.f, 1.f };
	pDeviceContext->ClearRenderTargetView(*renderTargetView, clearColor);

	// Do the full screen copy
	pDeviceContext->VSSetShader(*fullscreenVS, nullptr, 0);
	pDeviceContext->PSSetShader(*fullscreenPS, nullptr, 0);
	pDeviceContext->PSSetShaderResources(0, 1, &*renderTextureSRV);
	pDeviceContext->PSSetSamplers(0, 1, &*fullscreenSampler);

	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	D3D11_VIEWPORT viewport{ ViewportFromTexture(pBackBuffer) };
	
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->RSSetViewports(1, &viewport);
	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);

	pDeviceContext->Draw(4, 0);
}
