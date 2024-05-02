#include "Ch08_FontAtlas.h"

#include <tchar.h>

#include <cassert>

#include <algorithm>
#include <array>
#include <vector>

#include "util.h"

const D3D11_INPUT_ELEMENT_DESC FontAtlas::Vertex::desc[]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

// Note that while we're specifying our vertex positions in an arbitrary
// window space with (0,0) at the lower right, we're specifying our
// texel coords in normalized uv space, with (0,0) at the upper left
// and (1,1) at the loewr right.
const FontAtlas::Vertex FontQuad[] = {
	{ { 0, 300 + 64 }, { 0, 0 } }, // Upper left
	{ { 128, 300 + 64 }, { 1, 0 } }, // Upper right
	{ { 0, 300 - 64 }, { 0, 1 } }, // Lower left
	{ { 128, 300 - 64 }, { 1, 1 } }  // Lower right
};

const FontAtlas::Vertex GrayscaleQuad[] = {
	{ { 129, 300 + 64 }, { 0, 0 } }, // Upper left
	{ { 129 + 128, 300 + 64 }, { 1, 0 } }, // Upper right
	{ { 129, 300 - 64 }, { 0, 1 } }, // Lower left
	{ { 129 + 128, 300 - 64 }, { 1, 1 } }  // Lower right
};

void FontAtlas::LoadShaders()
{
	auto vsBytes = LoadFile(_T("TexturedQuad.vsc"));
	auto psBytes = LoadFile(_T("TexturedQuad.psc"));
	auto fontPsBytes = LoadFile(_T("Ch08_BasicFont.psc"));
	pDevice->CreateVertexShader(vsBytes.data(), vsBytes.size(), nullptr, &*vertexShader);
	pDevice->CreatePixelShader(psBytes.data(), psBytes.size(), nullptr, &*pixelShader);
	pDevice->CreatePixelShader(fontPsBytes.data(), fontPsBytes.size(), nullptr, &*basicFontPS);

	pDevice->CreateInputLayout(
		FontAtlas::Vertex::desc,
		countof(FontAtlas::Vertex::desc),
		vsBytes.data(),
		vsBytes.size(),
		&*inputLayout
	);
}

void FontAtlas::CreateFontTextures()
{
	// Set up for creating the original RGBA atlas from the loader
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroInitialize(texDesc);
	texDesc.Width = fontData.bitmapInfo.bmiHeader.biWidth;
	texDesc.Height = fontData.bitmapInfo.bmiHeader.biHeight;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA srd;
	srd.pSysMem = fontData.DIBits.data();
	srd.SysMemPitch = fontData.bitmapInfo.bmiHeader.biWidth * 4;
	srd.SysMemSlicePitch = 0;
	pDevice->CreateTexture2D(&texDesc, &srd, &*fontAtlas);

	// Create the grayscale font atlas
	std::vector<BYTE> grayscaleBits{ RemapFontBits() };

	texDesc.Format = DXGI_FORMAT_R8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	srd.pSysMem = grayscaleBits.data();
	srd.SysMemPitch = fontData.bitmapInfo.bmiHeader.biWidth;

	pDevice->CreateTexture2D(&texDesc, &srd, &*fontAtlasGrayscale);

	// Create resource views for both textures
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroInitialize(srvDesc);
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	D3D11_TEX2D_SRV& t2srv = srvDesc.Texture2D;
	t2srv.MipLevels = 1;
	t2srv.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(*fontAtlas, &srvDesc, &*fontAtlasSRV);

	srvDesc.Format = DXGI_FORMAT_R8_UNORM;
	pDevice->CreateShaderResourceView(*fontAtlasGrayscale, &srvDesc, &*fontAtlasGrayscaleSRV);
}

void FontAtlas::CreateVertexBuffers()
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroInitialize(bufferDesc);
	bufferDesc.ByteWidth = sizeof(FontQuad);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd;
	ZeroInitialize(srd);
	srd.pSysMem = FontQuad;

	pDevice->CreateBuffer(&bufferDesc, &srd, &*fontAtlasVB);

	srd.pSysMem = GrayscaleQuad;
	pDevice->CreateBuffer(&bufferDesc, &srd, &*grayscaleVB);
}

void FontAtlas::RescaleFontUVs()
{
	float cx = 1.f * fontData.bitmapInfo.bmiHeader.biWidth;
	float cy = 1.f * fontData.bitmapInfo.bmiHeader.biHeight;

	for (auto& [k, r] : fontData.glyphQuads)
	{
		r.left /= cx;
		r.right /= cx;
		r.top /= cy;
		r.bottom /= cy;
	}
}

std::vector<BYTE> FontAtlas::RemapFontBits()
{
	std::vector<BYTE> vb;
	constexpr float coefficients[4] = { 0.114f, 0.587f, 0.299f, 0.f };
	auto length = fontData.DIBits.size() / 4;
	vb.reserve(length);
	for (unsigned pos{ 0 }; pos < length; ++pos)
	{
		float acc{ 0.f };
		unsigned x = (pos) % fontData.bitmapInfo.bmiHeader.biWidth;
		unsigned y = (pos) / fontData.bitmapInfo.bmiHeader.biWidth;
		auto bp = fontData.DIBits.data() + (4 * pos);
		for (unsigned n{ 0 }; n < 4; ++n)
		{
			acc += coefficients[n] * (*(bp + n));
		}
		vb.emplace_back(BYTE(acc));
	}
	return vb;
}

void FontAtlas::CreateConstantBuffers()
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroInitialize(bufferDesc);
	bufferDesc.ByteWidth = aligned_size_16<D3D11_VIEWPORT>;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	pDevice->CreateBuffer(&bufferDesc, nullptr, &*viewportConstantBuffer);

	bufferDesc.ByteWidth = sizeof(float[4]);
	pDevice->CreateBuffer(&bufferDesc, nullptr, &*colorConstantBuffer);
}

void FontAtlas::Initialize()
{
	LoadShaders();

	fontData = FontLoader::LoadFont(_T("Calibri"), 19);
	CreateFontTextures();
	//RescaleFontUVs();
	CreateConstantBuffers();
	CreateVertexBuffers();

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroInitialize(samplerDesc);
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.f;
	pDevice->CreateSamplerState(&samplerDesc, &*samplerState);

	enableUpdate = true;
}

void FontAtlas::Update(double elapsed)
{
	if (!enableUpdate)
		Initialize();
}

void FontAtlas::RenderString(POINTF topleft, LPCTSTR lpsz, unsigned nChars, const float color[4])
{
	assert(lpsz);
	assert(nChars);

	ComPtr<ID3D11Buffer> vb;
	std::vector<Vertex> verts;
	verts.reserve(2 * nChars + 2);

	const float bmpx = fontData.bitmapInfo.bmiHeader.biHeight * 1.f;
	const float bmpy = fontData.bitmapInfo.bmiHeader.biWidth * 1.f;

	auto data = fontData.glyphQuads[*lpsz];
	float height = data.bottom - data.top;

	verts.emplace_back(Vertex{
		{ topleft.x, topleft.y + height },
		{ data.left / bmpx, data.bottom / bmpy }
	});
	verts.emplace_back(Vertex{
		{ topleft.x, topleft.y },
		{ data.left / bmpx, data.top / bmpy }
	});

	for (unsigned n{ 0 }; n < nChars; ++n)
	{
		data = fontData.glyphQuads[lpsz[n]];
		topleft.x += data.right;
		height = data.bottom - data.top;
		verts.emplace_back(Vertex{
			{ topleft.x, topleft.y + height },
			{ data.right / bmpx, data.bottom / bmpy }
		});
		verts.emplace_back(Vertex{
			{ topleft.x, topleft.y },
			{ data.right / bmpx, data.top / bmpy }
		});
	}

	D3D11_BUFFER_DESC bufferDesc;
	ZeroInitialize(bufferDesc);
	bufferDesc.ByteWidth = verts.size() * sizeof(Vertex);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd;
	ZeroInitialize(srd);
	srd.pSysMem = verts.data();

	pDevice->CreateBuffer(&bufferDesc, &srd, &*vb);

	D3D11_MAPPED_SUBRESOURCE sub;
	pDeviceContext->Map(*colorConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	memcpy(sub.pData, color, sizeof(float[4]));
	pDeviceContext->Unmap(*colorConstantBuffer, 0);

	unsigned strides[]{ 0 };
	unsigned offsets[]{ 0 };
	std::array<ID3D11Buffer*, 1> buffers{ *vb };
	pDeviceContext->IASetVertexBuffers(0, 1, buffers.data(), strides, offsets);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->PSSetShader(*basicFontPS, nullptr, 0);
	pDeviceContext->PSSetShaderResources(0, 1, &*fontAtlasGrayscaleSRV);
	pDeviceContext->Draw(verts.size(), 0);
}

void FontAtlas::Render()
{
	if (!enableUpdate)
		return;

	pDeviceContext->ClearState();

	constexpr float bgColor[]{ 1.f, 1.f, 1.f, 1.f };
	constexpr float textColor[]{ 0.8f, 0.361f, 0.149f, 1.f };

	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, bgColor);

	D3D11_VIEWPORT viewport{ ViewportFromTexture(pBackBuffer) };

	std::array<ID3D11Buffer*, 1> vertexBuffers{
		*fontAtlasVB,
	};
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pDeviceContext->Map(*viewportConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &viewport, sizeof(D3D11_VIEWPORT));
	pDeviceContext->Unmap(*viewportConstantBuffer, 0);

	unsigned strides[]{ sizeof(FontAtlas::Vertex)};
	unsigned offsets[]{ 0 };
	pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers.data(), strides, offsets);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->IASetInputLayout(*inputLayout);

	pDeviceContext->RSSetViewports(1, &viewport);
	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);

	pDeviceContext->VSSetConstantBuffers(0, 1, &*viewportConstantBuffer);
	pDeviceContext->VSSetShader(*vertexShader, nullptr, 0);

	pDeviceContext->PSSetShader(*pixelShader, nullptr, 0);
	pDeviceContext->PSSetShaderResources(0, 1, &*fontAtlasSRV);
	pDeviceContext->PSSetSamplers(0, 1, &*samplerState);

	pDeviceContext->Draw(4, 0);

	vertexBuffers[0] = *grayscaleVB;
	pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers.data(), strides, offsets);
	pDeviceContext->PSSetShaderResources(0, 1, &*fontAtlasGrayscaleSRV);
	pDeviceContext->Draw(4, 0);

	POINTF hwtl{ 5.f, 25.f };
	RenderString(hwtl, _T("Hello World!"), 12, textColor);

	pSwapChain->Present(1, 0);
}
