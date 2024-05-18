#include "Ch10_KernelsAndFilters.h"

#include <tchar.h>

#include <filesystem>

#include "util.h"

struct KernelConstants
{
	SIZE kernelSize;
	SIZE kernelOffset;
};

float BoxBlur[3][3]{
	{ 1.0f, 1.0f, 1.0f },
	{ 1.0f, 1.0f, 1.0f }, 
	{ 1.0f, 1.0f, 1.0f }
};

float GaussianBlur3x3[3][3]{
	{ 1.0f, 2.0f, 1.0f },
	{ 2.0f, 4.0f, 2.0f },
	{ 1.0f, 2.0f, 1.0f },
};

float GaussianBlur5x5[5][5]{
	{ 1.0f, 4.0f, 6.0f, 4.0f, 1.0f },
	{ 4.0f, 16.f, 24.f, 16.f, 4.0f },
	{ 6.0f, 24.f, 36.f, 24.f, 6.0f },
	{ 4.0f, 16.f, 24.f, 16.f, 4.0f },
	{ 1.0f, 4.0f, 6.0f, 4.0f, 1.0f }
};

float GaussianBlur7x7[7][7]{
	{ 0.0f, 0.0f, 1.0f, 2.0f, 1.0f, 0.0f, 0.0f },
	{ 0.0f, 3.0f, 13.f, 22.f, 13.f, 3.0f, 0.0f },
	{ 1.0f, 13.f, 59.f, 97.f, 59.f, 13.f, 1.0f },
	{ 2.0f, 22.f, 97.f, 159.f, 97.f, 22.f, 2.0f },
	{ 1.0f, 13.f, 59.f, 97.f, 59.f, 13.f, 1.0f },
	{ 0.0f, 3.0f, 13.f, 22.f, 13.f, 3.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f, 2.0f, 1.0f, 0.0f, 0.0f },
};

float SobelX[3][3]{
	{ 1, 0, -1 },
	{ 2, 0, -2 },
	{ 1, 0, -1 }
};

float SobelY[3][3]{
	{  1,  2,  1 },
	{  0,  0,  0 },
	{ -1, -2, -1 }
};

struct Kernel {
	KernelConstants params;
	float (*weights)[];
} kernels[] = {
	{ { { 3, 3 }, { -1, -1 } }, BoxBlur },
	{ { { 3, 3 }, { -1, -1 } }, GaussianBlur3x3 },
	{ { { 5, 5 }, { -2, -2 } }, GaussianBlur5x5 },
	{ { { 7, 7 }, { -3, -3 } }, GaussianBlur7x7 },
	{ { { 3, 3 }, { -1, -1 } }, SobelX },
	{ { { 3, 3 }, { -1, -1 } }, SobelY },
};

const D3D11_INPUT_ELEMENT_DESC KernelFilters::Vertex::desc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

//const KernelFilters::Vertex KernelFilters::unfilteredVerts[]
//{
//
//};
//
//const KernelFilters::Vertex KernelFilters::filteredVerts[]
//{
//
//};

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
	namespace fs = std::filesystem;
	std::string path = ".\\assets";
	for (const auto& entry : fs::directory_iterator(path)) 
	{
		if (!entry.is_directory())
		{
			Image image;
			image.Load(entry.path().c_str());
			auto imageBytes = image.GetFrameData(0);

			D3D11_TEXTURE2D_DESC desc;
			ZeroInitialize(desc);
			desc.Width = image.Width();
			desc.Height = image.Height();
			desc.ArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			D3D11_SUBRESOURCE_DATA srd;
			ZeroInitialize(srd);
			srd.pSysMem = imageBytes.data();
			srd.SysMemPitch = 4 * image.Width();

			ComPtr<ID3D11Texture2D> texPtr;
			pDevice->CreateTexture2D(&desc, &srd, &*texPtr);
			if (texPtr)
				images.emplace_back(texPtr);
		}
	}
}

void KernelFilters::CreateConstantBuffers()
{
	D3D11_BUFFER_DESC desc;
	ZeroInitialize(desc);
	desc.ByteWidth = aligned_size_16<D3D11_VIEWPORT>;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	pDevice->CreateBuffer(&desc, nullptr, &*quadInputCBuffer);

	desc.ByteWidth = aligned_size_16<KernelConstants>;
	
	pDevice->CreateBuffer(&desc, nullptr, &*kernelShaderCBuffer);
}

void KernelFilters::CreateSamplers()
{
}

void KernelFilters::CreateVertexBuffers()
{

}

void KernelFilters::Initialize()
{
	LoadShaders();
	LoadTextures();
	CreateConstantBuffers();
	CreateSamplers();
	CreateVertexBuffers();
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
