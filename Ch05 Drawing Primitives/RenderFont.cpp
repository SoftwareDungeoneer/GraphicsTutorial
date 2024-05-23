#include "RenderFont.h"

#include <cassert>
#include <vector>

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

	CreateBlendState();
	CreateSamplerState();
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

void RenderFont::AllocateBufferSpace(unsigned nChars)
{
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

void RenderFont::RenderString(POINTF topleft, LPCTSTR lpsz, unsigned nChars, const ColorF color)
{

}
