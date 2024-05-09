#include "Ch09_DepthBuffer.h"

#include <tchar.h>

#include "util.h"

const D3D11_INPUT_ELEMENT_DESC DepthBuffer::Vertex::desc[]{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const std::array<DepthBuffer::Vertex, 9> DepthBuffer::verts = {
	{
		{ { -0.718f,  0.057f, 1.f }, { 0, 0, 1, 1 } },
		{ {  0.876f, -0.105f, 0.f }, { 0, 0, 1, 1 } },
		{ {  0.738f, -0.452f, 0.f }, { 0, 0, 1, 1 } },

		{ {  0.309f, -0.650f, 1.f }, { 1, 0, 1, 1 } },
		{ { -0.347f,  0.811f, 0.f }, { 1, 0, 1, 1 } },
		{ {  0.022f,  0.865f, 0.f }, { 1, 0, 1, 1 } },

		{ {  0.408f,  0.593f, 1.f }, { .1f, 1, .2f, 1 } },
		{ { -0.529f, -0.706f, 0.f }, { .1f, 1, .2f, 1 } },
		{ { -0.760f, -0.413f, 0.f }, { .1f, 1, .2f, 1 } },
	}
};

void DepthBuffer::Initialize()
{
	auto vsBytes = LoadFile(_T("Ch09_Depth2D.vsc"));
	auto psBytes = LoadFile(_T("CH09_Depth2D.psc"));
	auto fullscreenVsBytes = LoadFile(_T("FullscreenQuad.vsc"));
	auto fullscreenPsBytes = LoadFile(_T("TexturedQuad.psc"));
	auto depthRenderPSBytes = LoadFile(_T("DepthToGray.psc"));

	pDevice->CreateVertexShader(vsBytes.data(), vsBytes.size(), nullptr, &*depthVertexShader);
	pDevice->CreateVertexShader(fullscreenVsBytes.data(), fullscreenVsBytes.size(), nullptr, &*fullscreenVS);
	pDevice->CreatePixelShader(psBytes.data(), psBytes.size(), nullptr, &*depthPixelShader);
	pDevice->CreatePixelShader(fullscreenPsBytes.data(), fullscreenPsBytes.size(), nullptr, &*fullscreenPS);
	pDevice->CreatePixelShader(depthRenderPSBytes.data(), depthRenderPSBytes.size(), nullptr, &*depthPS);
	pDevice->CreateInputLayout(
		Vertex::desc, 
		countof(Vertex::desc),
		vsBytes.data(),
		vsBytes.size(),
		&*inputLayout
	);

	CreateVertexBuffer();
	CreateTextureRenderTarget();
	CreateDepthStencilAndState();

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroInitialize(samplerDesc);
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.f;
	pDevice->CreateSamplerState(&samplerDesc, &*fullscreenSampler);
}

void DepthBuffer::CreateVertexBuffer()
{
	D3D11_BUFFER_DESC desc;
	ZeroInitialize(desc);
	desc.ByteWidth = verts.size() * sizeof(Vertex);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd;
	ZeroInitialize(srd);
	srd.pSysMem = verts.data();

	pDevice->CreateBuffer(&desc, &srd, &*vertexBuffer);
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

void DepthBuffer::CreateDepthStencilAndState()
{
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroInitialize(texDesc);
	texDesc.Width = 1024;
	texDesc.Height = 768;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc = { 1, 0 };
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	pDevice->CreateTexture2D(&texDesc, nullptr, &*depthTexture);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroInitialize(dsvDesc);
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	pDevice->CreateDepthStencilView(*depthTexture, &dsvDesc, &*depthStencilView);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroInitialize(srvDesc);
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D = { 0, 1 };

	pDevice->CreateShaderResourceView(*depthTexture, &srvDesc, &*depthStencilSRV);

	D3D11_DEPTH_STENCIL_DESC stateDesc;
	ZeroInitialize(stateDesc);
	stateDesc.DepthEnable = TRUE;
	stateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	stateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	stateDesc.StencilEnable = FALSE;
	
	pDevice->CreateDepthStencilState(&stateDesc, &*depthStencilState);
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

	constexpr float clearColor[4]{ 0.f, 0.f, 0.f, 1.f };
	pDeviceContext->ClearRenderTargetView(*renderTargetView, clearColor);
	pDeviceContext->ClearDepthStencilView(
		*depthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.f,
		0
	);
	
	D3D11_VIEWPORT viewport{ ViewportFromTexture(renderTexture) };

	std::array<unsigned, 1> strides{ sizeof(Vertex) };
	std::array<unsigned, 1> offsets{ 0 };
	pDeviceContext->IASetInputLayout(*inputLayout);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->IASetVertexBuffers(0, 1, &*vertexBuffer, strides.data(), offsets.data());

	pDeviceContext->RSSetViewports(1, &viewport);

	pDeviceContext->OMSetRenderTargets(1, &*renderTargetView, *depthStencilView);

	pDeviceContext->VSSetShader(*depthVertexShader, nullptr, 0);
	pDeviceContext->PSSetShader(*depthPixelShader, nullptr, 0);

	pDeviceContext->Draw(verts.size(), 0);

	// We need to unbind the render targets and depth target before using them 
	// as SRVs
	pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	// Do the full screen copy
	pDeviceContext->VSSetShader(*fullscreenVS, nullptr, 0);
	pDeviceContext->PSSetShader(*fullscreenPS, nullptr, 0);
	pDeviceContext->PSSetShaderResources(0, 1, &*renderTextureSRV);
	pDeviceContext->PSSetSamplers(0, 1, &*fullscreenSampler);

	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	float whiteness[4] = { 1.f, 1.f, 1.f, 1.f };
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, whiteness);
	viewport = ViewportFromTexture(pBackBuffer);
	viewport.Width /= 2;
	viewport.Height /= 2;
	
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->RSSetViewports(1, &viewport);
	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);

	pDeviceContext->Draw(4, 0);

	viewport.TopLeftX = viewport.Width;
	pDeviceContext->RSSetViewports(1, &viewport);
	pDeviceContext->PSSetShaderResources(0, 1, &*depthStencilSRV);
	pDeviceContext->PSSetShader(*depthPS, nullptr, 0);
	pDeviceContext->Draw(4, 0);

	pSwapChain->Present(1, 0);
}
