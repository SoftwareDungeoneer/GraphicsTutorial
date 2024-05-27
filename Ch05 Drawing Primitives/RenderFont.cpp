#include "RenderFont.h"

#include <tchar.h>

#include <cassert>
#include <vector>

#include "util.h"

struct GlyphVertex
{
	float Pos2D[2]{ 0, 0 };
	float Texcoord[2]{ 0, 0 };

	static const D3D11_INPUT_ELEMENT_DESC desc[];
};

const D3D11_INPUT_ELEMENT_DESC GlyphVertex::desc[]
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

RenderFont::RenderFont(
	ComPtr<ID3D11Device> _device,
	ComPtr<ID3D11DeviceContext> _context):
	pDevice(_device),
	pContext(_context)
{
	assert(_device);
	assert(_context);

	LoadShaders();
	CreateConstantBuffers();
	CreateBlendState();
	CreateSamplerState();
}

void RenderFont::LoadShaders()
{
	assert(pDevice);

	auto vsBytes = LoadFile(_T("TexturedQuad.vsc"));
	auto psBytes = LoadFile(_T("Ch08_BasicFont.psc"));
	pDevice->CreateVertexShader(vsBytes.data(), vsBytes.size(), nullptr, &*vertexShader);
	pDevice->CreatePixelShader(psBytes.data(), psBytes.size(), nullptr, &*pixelShader);

	pDevice->CreateInputLayout(
		GlyphVertex::desc,
		countof(GlyphVertex::desc),
		vsBytes.data(),
		vsBytes.size(),
		&*inputLayout
	);
}

void RenderFont::CreateConstantBuffers()
{
	assert(pDevice);

	D3D11_BUFFER_DESC desc;
	ZeroInitialize(desc);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(ColorF);

	pDevice->CreateBuffer(&desc, nullptr, &*colorConstantBuffer);

	desc.ByteWidth = aligned_size_16<D3D11_VIEWPORT>;
	pDevice->CreateBuffer(&desc, nullptr, &*viewportConstantBuffer);
}

void RenderFont::CreateBlendState()
{	
	///////////////////////////////////////////////////////////////////////////
	// Create alpha blend blend desc
	///////////////////////////////////////////////////////////////////////////
	assert(pDevice);

	D3D11_BLEND_DESC blendDesc;
	D3D11_RENDER_TARGET_BLEND_DESC rtbd
	{
		TRUE, // Blend enable
		D3D11_BLEND_SRC_ALPHA,
		D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_OP_ADD,
		D3D11_BLEND_ZERO,
		D3D11_BLEND_DEST_ALPHA,
		D3D11_BLEND_OP_ADD,
		D3D11_COLOR_WRITE_ENABLE_ALL
	};
	ZeroInitialize(blendDesc);
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0] = rtbd;
	pDevice->CreateBlendState(&blendDesc, &*alphaEnableBlendState);
}

void RenderFont::CreateSamplerState()
{
	///////////////////////////////////////////////////////////////////////////
	// Create point sampler
	///////////////////////////////////////////////////////////////////////////
	assert(pDevice);

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroInitialize(samplerDesc);
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.f;
	pDevice->CreateSamplerState(&samplerDesc, &*pointSampler);
}

void RenderFont::CreateFontTexture()
{
	assert(pDevice);

	std::vector<BYTE> grayscaleBits{ RemapFontBits() };
	RescaleFontUVs();

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroInitialize(texDesc);
	texDesc.Width = fontData.bitmapInfo.bmiHeader.biWidth;
	texDesc.Height = fontData.bitmapInfo.bmiHeader.biHeight;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA srd;
	srd.pSysMem = grayscaleBits.data();
	srd.SysMemPitch = fontData.bitmapInfo.bmiHeader.biWidth;
	srd.SysMemSlicePitch = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroInitialize(srvDesc);
	srvDesc.Format = DXGI_FORMAT_R8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	D3D11_TEX2D_SRV& t2srv = srvDesc.Texture2D;
	t2srv.MipLevels = 1;
	t2srv.MostDetailedMip = 0;

	pDevice->CreateTexture2D(&texDesc, &srd, &*fontTexture);
	pDevice->CreateShaderResourceView(*fontTexture, &srvDesc, &*fontSRV);
}

std::vector<BYTE> RenderFont::RemapFontBits()
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

void RenderFont::RescaleFontUVs()
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

void RenderFont::AllocateBufferSpace(unsigned nChars)
{
	assert(pDevice);

	if (nChars <= cchHighWater)
		return;

	cchHighWater = nChars;

	///////////////////////////////////////////////////////////////////////////
	// Create index data for index buffer
	///////////////////////////////////////////////////////////////////////////
	constexpr short indexTemplate[]{ 0, 1, 2, 2, 1, 3 };
	std::vector<short> indices(6 * nChars);
	for (unsigned n{ 0 }; n < nChars; ++n)
	{
		for (unsigned j{ 0 }; j < countof(indexTemplate); ++j)
		{
			indices[(6 * n) + j] = indexTemplate[j] + (4 * n);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Create the underlying d3d index buffer
	///////////////////////////////////////////////////////////////////////////
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroInitialize(indexBufferDesc);
	indexBufferDesc.ByteWidth = static_cast<unsigned>(indices.size() * sizeof(short));
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexBufferSRD{ indices.data(), 0, 0 };
	
	pDevice->CreateBuffer(&indexBufferDesc, &indexBufferSRD, &*indexBuffer);

	///////////////////////////////////////////////////////////////////////////
	// Create underlying vertex buffer
	///////////////////////////////////////////////////////////////////////////
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroInitialize(vertexBufferDesc);
	vertexBufferDesc.ByteWidth = static_cast<unsigned>(4 * nChars * sizeof(GlyphVertex));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	pDevice->CreateBuffer(&vertexBufferDesc, nullptr, &*vertexBuffer);
}

void RenderFont::LoadFont(LPCTSTR fontFace, int height)
{
	fontData = FontLoader::LoadFont(fontFace, height);
	CreateFontTexture();
}

void RenderFont::LayoutString(POINTF topleft, LPCTSTR lpsz, unsigned nChars)
{
	assert(pContext);
	assert(nChars <= cchHighWater);

	std::vector<GlyphVertex> verts(4 * cchHighWater, GlyphVertex{ {0.f, 0.f}, {0.f, 0.f} });
	
	const float bmpx = fontData.bitmapInfo.bmiHeader.biHeight * 1.f;
	const float bmpy = fontData.bitmapInfo.bmiHeader.biWidth * 1.f;

	for (unsigned n{ 0 }; n < nChars; ++n)
	{
		auto data = fontData.glyphQuads[lpsz[n]];
		float height = data.bottom - data.top;
		float width = data.right - data.left;
		unsigned base = 4 * n;
		verts[base] = GlyphVertex{ // Upper left vert
			{ topleft.x, topleft.y },
			{ data.left / bmpx, data.top / bmpy }
		};
		verts[base + 1] = GlyphVertex{ // Upper right vert
			{ topleft.x + width, topleft.y },
			{ data.right / bmpx, data.top / bmpy }
		};
		verts[base + 2] = GlyphVertex{ // Lower left vert
			{ topleft.x, topleft.y - height },
			{ data.left / bmpx, data.bottom / bmpy }
		};
		verts[base + 3] = GlyphVertex{ // lower right vert
			{ topleft.x + width, topleft.y - height },
			{ data.right / bmpx, data.bottom / bmpy }
		};
		topleft.x += width;
	}

	pContext->UpdateSubresource(*vertexBuffer, 0, nullptr, verts.data(), 0, 0);
}

void RenderFont::RenderString(POINTF topleft, LPCTSTR lpsz, unsigned nChars, const ColorF color)
{
	assert(pContext);

	// Do CPU side string layout and buffer fixup
	AllocateBufferSpace(nChars);
	LayoutString(topleft, lpsz, nChars);

	D3D11_VIEWPORT viewport;
	unsigned numViewports{ 1 };
	pContext->RSGetViewports(&numViewports, &viewport);

	pContext->UpdateSubresource(*viewportConstantBuffer, 0, nullptr, &viewport, 0, 0);
	pContext->UpdateSubresource(*colorConstantBuffer, 0, nullptr, color.channels, 0, 0);

	unsigned strides[]{ sizeof(GlyphVertex) };
	unsigned offsets[]{ 0 };

	// Set up Input Assember
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->IASetIndexBuffer(*indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	pContext->IASetVertexBuffers(0, 1, &*vertexBuffer, strides, offsets);

	// Rasterizer Should be set up by whatever rendering was done earlier
	
	// Output merger already has render target set by calling stage, back up and set
	// blend state
	ComPtr<ID3D11BlendState> oldBlendState;
	float blendFactors[4];
	unsigned sampleMask;
	pContext->OMGetBlendState(&*oldBlendState, blendFactors, &sampleMask);
	pContext->OMSetBlendState(*alphaEnableBlendState, blendFactors, sampleMask);

	// Set up vertex shader
	pContext->VSSetShader(*vertexShader, nullptr, 0);
	pContext->VSSetConstantBuffers(0, 1, &*viewportConstantBuffer);

	// Set up pixel shader
	pContext->PSSetShader(*pixelShader, nullptr, 0);
	pContext->PSSetConstantBuffers(0, 1, &*colorConstantBuffer);
	pContext->PSSetShaderResources(0, 1, &*fontSRV);
	pContext->PSSetSamplers(0, 1, &*pointSampler);

	// Issue batch
	pContext->DrawIndexed(6 * nChars, 0, 0);

	// Restore OM state
	pContext->OMSetBlendState(*oldBlendState, blendFactors, sampleMask);
}
