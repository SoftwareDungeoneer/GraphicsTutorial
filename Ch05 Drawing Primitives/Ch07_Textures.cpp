#include "Ch07_Textures.h"

#include <tchar.h>

#include <array>

#include "util.h"

const D3D11_INPUT_ELEMENT_DESC Textures::Vertex::desc[]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

// Note that while we're specifying our vertex positions in an arbitrary
// window space with (0,0) at the lower right, we're specifying our
// texel coords in normalized uv space, with (0,0) at the upper left
// and (1,1) at the loewr right.
const Textures::Vertex QuadVertices[] = {
	{ { 400 - 128, 300 + 128 }, { 0, 0 } }, // Upper left
	{ { 400 + 128, 300 + 128 }, { 1, 0 } }, // Upper right
	{ { 400 - 128, 300 - 128 }, { 0, 1 } }, // Lower left
	{ { 400 + 128, 300 - 128 }, { 1, 1 } }  // Lower right
};

void Textures::Initialize()
{
	starImage = std::make_unique<Image>();
	starImage->Load(_T("WhiteStarOnGreen.png"));
	auto imageBytes = starImage->GetFrameData(0);

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

	D3D11_BUFFER_DESC bufferDesc;
	ZeroInitialize(bufferDesc);
	bufferDesc.ByteWidth = sizeof(QuadVertices);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd;
	ZeroInitialize(srd);
	srd.pSysMem = QuadVertices;

	pDevice->CreateBuffer(&bufferDesc, &srd, &*vertexBuffer);

	bufferDesc.ByteWidth = aligned_size_16<D3D11_VIEWPORT>;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	pDevice->CreateBuffer(&bufferDesc, nullptr, &*viewportConstantBuffer);

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroInitialize(texDesc);
	texDesc.Width = starImage->Width();
	texDesc.Height = starImage->Height();
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	
	srd.pSysMem = imageBytes.data();
	srd.SysMemPitch = starImage->Width() * 4;
	pDevice->CreateTexture2D(&texDesc, &srd, &*texture);

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroInitialize(samplerDesc);
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.f;
	pDevice->CreateSamplerState(&samplerDesc, &*samplerState);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroInitialize(srvDesc);
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	D3D11_TEX2D_SRV& t2srv = srvDesc.Texture2D;
	t2srv.MipLevels = 1;
	t2srv.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(*texture, &srvDesc, &*textureSRV);

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
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pDeviceContext->Map(*viewportConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &viewport, sizeof(D3D11_VIEWPORT));
	pDeviceContext->Unmap(*viewportConstantBuffer, 0);

	unsigned strides[]{ sizeof(Textures::Vertex)};
	unsigned offsets[]{ 0 };
	pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers.data(), strides, offsets);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->IASetInputLayout(*inputLayout);

	pDeviceContext->RSSetViewports(1, &viewport);
	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);

	pDeviceContext->VSSetConstantBuffers(0, 1, &*viewportConstantBuffer);
	pDeviceContext->VSSetShader(*vertexShader, nullptr, 0);

	pDeviceContext->PSSetShader(*pixelShader, nullptr, 0);
	pDeviceContext->PSSetShaderResources(0, 1, &*textureSRV);
	pDeviceContext->PSSetSamplers(0, 1, &*samplerState);

	pDeviceContext->Draw(4, 0);

	pSwapChain->Present(1, 0);
}
