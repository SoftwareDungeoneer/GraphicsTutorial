#include "Ch10_KernelsAndFilters.h"

#include <tchar.h>

#include "util.h"

struct KernelConstants
{
	SIZE kernelSize;
	SIZE textureSize;
	SIZE kernelOffset;
};

const D3D11_INPUT_ELEMENT_DESC KernelFilters::Vertex::desc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

void KernelFilters::LoadShaders()
{
	auto vsBytes = LoadFile(_T("TexturedQuad.vsc"));
	auto psUnfiltered = LoadFile(_T("TexturedQuad.psc"));
	auto psFilterBytes = LoadFile(_T("Ch10_ImageProcessing.psc"));

	pDevice->CreateVertexShader(vsBytes.data(), vsBytes.size(), nullptr, &*quadVertexShader);
	pDevice->CreatePixelShader(psUnfiltered.data(), psUnfiltered.size(), nullptr, &*unfilteredShader);
	pDevice->CreatePixelShader(psFilterBytes.data(), psFilterBytes.size(), nullptr, &*kernelShader);

	pDevice->CreateInputLayout(
		Vertex::desc,
		countof(Vertex::desc),
		vsBytes.data(),
		vsBytes.size(),
		&*quadInputLayout
	);
}

void KernelFilters::LoadTextures()
{
}

void KernelFilters::CreateConstantBuffers()
{
	D3D11_BUFFER_DESC desc;
	ZeroInitialize(desc);
	desc.ByteWidth = aligned_size_16<D3D11_VIEWPORT>;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	pDevice->CreateBuffer(&desc, nullptr, &*quadInputCBuffer);

	desc.ByteWidth = 
}

void KernelFilters::CreateSamplers()
{
}

void KernelFilters::Initialize()
{
	LoadShaders();
	LoadTextures();
	CreateConstantBuffers();
	CreateSamplers();
}

void KernelFilters::Update(double elapsed)
{
	if (!enableUpdate)
		Initialize();
	enableUpdate = true;
}

void KernelFilters::Render()
{
	pDeviceContext->ClearState();

	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	float whiteness[4] = { 1.f, 1.f, 1.f, 1.f };
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, whiteness);

	D3D11_VIEWPORT viewport = ViewportFromTexture(pBackBuffer);

	// Draw here

	pSwapChain->Present(1, 0);
}
