#include "Ch10_KernelsAndFilters.h"

#include <tchar.h>

#include <filesystem>

#include "util.h"

struct KernelConstants
{
	SIZE kernelSize;
	SIZE kernelOffset;
};

float Unfiltered[1][1]{ { 1.0f } };

float Identity[3][3]{
	{ 0.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }
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

namespace
{
	struct Kernel {
		KernelConstants params;
		float(*weights)[];
	} kernels[] = {
		{ { { 1, 1 }, {  0,  0 } }, Unfiltered },
		{ { { 3, 3 }, { -1, -1 } }, Identity },
		{ { { 3, 3 }, { -1, -1 } }, BoxBlur },
		{ { { 3, 3 }, { -1, -1 } }, GaussianBlur3x3 },
		{ { { 5, 5 }, { -2, -2 } }, GaussianBlur5x5 },
		{ { { 7, 7 }, { -3, -3 } }, GaussianBlur7x7 },
		{ { { 3, 3 }, { -1, -1 } }, SobelX },
		{ { { 3, 3 }, { -1, -1 } }, SobelY },
	};
}

//static_assert(KernelFilters::KernelSel::COUNT == countof(kernels));

const D3D11_INPUT_ELEMENT_DESC KernelFilters::Vertex::desc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

void KernelFilters::NotifyKeyUp(unsigned key)
{
	switch (key)
	{
	case VK_LEFT:
		if (selectedTexture == inputTextures.begin())
			selectedTexture = inputTextures.end();
		--selectedTexture;
		break;

	case VK_RIGHT:
		++selectedTexture;
		if (selectedTexture == inputTextures.end())
			selectedTexture = inputTextures.begin();
		break;

	case VK_UP:
		if (activeKernel == KernelSel::Unfiltered)
			activeKernel = KernelSel::COUNT;
		activeKernel = static_cast<KernelSel>(activeKernel - 1);
		UpdateKernelTexture();
		break;

	case VK_DOWN:
		activeKernel = static_cast<KernelSel>(activeKernel + 1);
		if (activeKernel == KernelSel::COUNT)
			activeKernel = KernelSel::Unfiltered;
		UpdateKernelTexture();
		break;
	}
}

void KernelFilters::UpdateKernelTexture()
{
	D3D11_MAPPED_SUBRESOURCE map;
	ZeroInitialize(map);
	HRESULT hr = pDeviceContext->Map(*kernelTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);

	Kernel& k = kernels[activeKernel];
	for (int y = 0; y < k.params.kernelSize.cy; ++y)
	{
		unsigned kernelRowPitch = k.params.kernelSize.cx * sizeof(float);
		float* pSource = *k.weights + k.params.kernelSize.cx * y;
		void* pDest = reinterpret_cast<char*>(map.pData) + map.RowPitch * y;
		memcpy(pDest, pSource, sizeof(float) * k.params.kernelSize.cx);
	}

	pDeviceContext->Unmap(*kernelTexture, 0);

	pDeviceContext->UpdateSubresource(*kernelShaderCBuffer, 0, nullptr, &k.params, 0, 0);
}

std::string KernelFilters::ActiveKernelString()
{
	switch (activeKernel)
	{
	case Unfiltered: return "Unfiltered";
	case Identity: return "Identity";
	case kBoxBlur: return "Box Blur";
	case kGauss3: return "Gaussian 3x3";
	case kGauss5: return "Gaussian 5x5";
	case kGauss7: return "Gaussian 7x7";
	case kSobelX: return "Sobel X";
	case kSobelY: return "Sobel Y";

	default:
		return "Invalid value";
	}
}
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
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
				ZeroInitialize(srvDesc);
				srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				D3D11_TEX2D_SRV& t2srv = srvDesc.Texture2D;
				t2srv.MipLevels = 1;
				t2srv.MostDetailedMip = 0;

				ComPtr<ID3D11ShaderResourceView> srvPtr;
				pDevice->CreateShaderResourceView(*texPtr, &srvDesc, &*srvPtr);

				//auto pair = std::make_pair(texPtr, srvPtr);
				inputTextures.emplace_back(texPtr, srvPtr);
			}
		}
	}
	selectedTexture = inputTextures.begin();

	D3D11_TEXTURE2D_DESC desc{
		16, 16, 1, 1,
		DXGI_FORMAT_R32_FLOAT,
		{ 1, 0 },
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_SHADER_RESOURCE,
		D3D11_CPU_ACCESS_WRITE
	};
	pDevice->CreateTexture2D(&desc, nullptr, &*kernelTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroInitialize(srvDesc);
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	D3D11_TEX2D_SRV& t2srv = srvDesc.Texture2D;
	t2srv.MipLevels = 1;
	t2srv.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(*kernelTexture, &srvDesc, &*kernelSrv);

	UpdateKernelTexture();
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
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroInitialize(samplerDesc);
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.f;
	pDevice->CreateSamplerState(&samplerDesc, &*linearSampler);
}

void KernelFilters::CreateVertexBuffers()
{
	D3D11_BUFFER_DESC desc;
	ZeroInitialize(desc);
	desc.ByteWidth = sizeof(unfilteredVerts);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd;
	ZeroInitialize(srd);
	srd.pSysMem = unfilteredVerts;

	pDevice->CreateBuffer(&desc, &srd, &*unfilteredVertBuffer);
	
	srd.pSysMem = filteredVerts;
	pDevice->CreateBuffer(&desc, &srd, &*outputVertBuffer);

	ResizeNotify();
}

void KernelFilters::Initialize()
{
	CreateConstantBuffers();
	LoadShaders();
	LoadTextures();
	CreateSamplers();
	CreateVertexBuffers();
}

void KernelFilters::ResizeNotify()
{
	int mid_y = windowHeight / 2;
	float top_y = mid_y + 256.f;
	float bottom_y = mid_y - 256.f;
	float left = 0;
	float right = 512;

	unfilteredVerts[0] = Vertex{ { left, top_y }, { 0, 0 } };
	unfilteredVerts[1] = Vertex{ { right, top_y }, { 1, 0 } };
	unfilteredVerts[2] = Vertex{ { left, bottom_y }, { 0, 1 } };
	unfilteredVerts[3] = Vertex{ { right, bottom_y }, { 1, 1, } };

	left = right;
	right += 512;
	filteredVerts[0] = Vertex{ { left, top_y }, { 0, 0 } };
	filteredVerts[1] = Vertex{ { right, top_y }, { 1, 0 } };
	filteredVerts[2] = Vertex{ { left, bottom_y }, { 0, 1 } };
	filteredVerts[3] = Vertex{ { right, bottom_y }, { 1, 1, } };

	if (unfilteredVertBuffer)
		pDeviceContext->UpdateSubresource(*unfilteredVertBuffer, 0, nullptr, unfilteredVerts, 0, 0);
	if (outputVertBuffer)
		pDeviceContext->UpdateSubresource(*outputVertBuffer, 0, nullptr, filteredVerts, 0, 0);
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
	float bgColor[4] = { 0.f, 0.f, 0.f, 1.f };
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, bgColor);

	D3D11_VIEWPORT viewport = ViewportFromTexture(pBackBuffer);
	pDeviceContext->UpdateSubresource(*quadInputCBuffer, 0, nullptr, &viewport, 0, 0);

	// Draw here
	unsigned strides[]{ sizeof(Vertex) };
	unsigned offsets[]{ 0 };

	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->IASetInputLayout(*quadInputLayout);
	pDeviceContext->IASetVertexBuffers(0, 1, &*unfilteredVertBuffer, strides, offsets);

	pDeviceContext->RSSetViewports(1, &viewport);

	D3D11_RASTERIZER_DESC rsd;
	ZeroInitialize(rsd);
	rsd.FillMode = D3D11_FILL_SOLID;
	rsd.CullMode = D3D11_CULL_NONE;
	rsd.DepthBias = 0;
	rsd.DepthBiasClamp = 0.0f;

	ComPtr<ID3D11RasterizerState> rstate;
	pDevice->CreateRasterizerState(&rsd, &*rstate);

	pDeviceContext->RSSetState(*rstate);

	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);

	pDeviceContext->VSSetShader(*quadVertexShader, nullptr, 0);
	pDeviceContext->VSSetConstantBuffers(0, 1, &*quadInputCBuffer);

	pDeviceContext->PSSetShader(*unfilteredShader, nullptr, 0);
	pDeviceContext->PSSetShaderResources(0, 1, &*(selectedTexture->second));
	pDeviceContext->PSSetSamplers(0, 1, &*linearSampler);

	pDeviceContext->Draw(4, 0);

	ID3D11ShaderResourceView* textures[] = { *(selectedTexture->second), *kernelSrv };
	pDeviceContext->IASetVertexBuffers(0, 1, &*outputVertBuffer, strides, offsets);

	pDeviceContext->PSSetShader(*kernelShader, nullptr, 0);
	pDeviceContext->PSSetShaderResources(1, 1, &*kernelSrv);
	pDeviceContext->PSSetConstantBuffers(0, 1, &*kernelShaderCBuffer);

	pDeviceContext->Draw(4, 0);

	// Debug draw
	POINT kernelLabelPt{ filteredVerts[0].Pos[0], filteredVerts[0].Pos[1] + 20};
	pDebugDraw->DrawTextSS(kernelLabelPt, ActiveKernelString());

	pDebugDraw->Render();

	pSwapChain->Present(1, 0);
}
