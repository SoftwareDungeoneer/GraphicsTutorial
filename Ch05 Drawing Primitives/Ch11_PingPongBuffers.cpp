#include "Ch11_PingPongBuffers.h"

#include <tchar.h>

#include "util.h"

struct KernelConstants
{
	SIZE kernelSize;
	SIZE kernelOffset;
};

float GaussX[1][3]{ 1.f, 2.f, 1.f };
float GaussY[3][1]{ {1.f}, {2.f}, {1.f} };

struct Kernel {
	KernelConstants params;
	float(*weights)[];
} kernels[] = {
	{ { { 1, 3 }, {  -1,  0 } }, GaussX },
	{ { { 3, 1 }, {   0, -1 } }, GaussY },
};

//static_assert(PingPongBuffers::KernelSel::COUNT == countof(kernels));

const D3D11_INPUT_ELEMENT_DESC PingPongBuffers::Vertex::desc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

void PingPongBuffers::UpdateKernelTexture()
{
	D3D11_TEXTURE2D_DESC desc;
	kernelTexture->GetDesc(&desc);

	D3D11_MAPPED_SUBRESOURCE map;
	ZeroInitialize(map);
	HRESULT hr = pDeviceContext->Map(*kernelTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);

	memset(map.pData, 0, map.RowPitch * desc.Height);
	memcpy(map.pData, GaussX, sizeof(GaussX));
	void* pDest = reinterpret_cast<char*>(map.pData) + map.RowPitch;
	*((float*)(pDest)) = GaussY[1][0];
	pDest = reinterpret_cast<char*>(pDest) + map.RowPitch;
	*((float*)(pDest)) = GaussY[2][0];

	pDeviceContext->Unmap(*kernelTexture, 0);
}

void PingPongBuffers::LoadShaders()
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

void PingPongBuffers::LoadTextures()
{
	std::wstring path = L".\\assets\\Dichondra-grass-256x256.png";
	Image image;
	image.Load(path.c_str());
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

	pDevice->CreateTexture2D(&desc, &srd, &*inputTexture);
	if (inputTexture)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroInitialize(srvDesc);
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		D3D11_TEX2D_SRV& t2srv = srvDesc.Texture2D;
		t2srv.MipLevels = 1;
		t2srv.MostDetailedMip = 0;

		pDevice->CreateShaderResourceView(*inputTexture, &srvDesc, &*inputSRV);
	}

	D3D11_TEXTURE2D_DESC kernelDesc{
		16, 16, 1, 1,
		DXGI_FORMAT_R32_FLOAT,
		{ 1, 0 },
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_SHADER_RESOURCE,
		D3D11_CPU_ACCESS_WRITE
	};
	pDevice->CreateTexture2D(&kernelDesc, nullptr, &*kernelTexture);

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

void PingPongBuffers::CreateConstantBuffers()
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

void PingPongBuffers::CreateSamplers()
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

void PingPongBuffers::CreateVertexBuffers()
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

void PingPongBuffers::CreatePingPongBuffers()
{
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroInitialize(texDesc);
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroInitialize(srvDesc);
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroInitialize(rtvDesc);
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	for (unsigned n{ 0 }; n < countof(pingPongBuffers); ++n)
	{
		pDevice->CreateTexture2D(&texDesc, nullptr, &*pingPongBuffers[n].texture);
		pDevice->CreateShaderResourceView(*pingPongBuffers[n].texture, &srvDesc, &*pingPongBuffers[n].srv);
		pDevice->CreateRenderTargetView(*pingPongBuffers[n].texture, &rtvDesc, &*pingPongBuffers[n].rtv);
	}
}
void PingPongBuffers::Initialize()
{
	CreateConstantBuffers();
	LoadShaders();
	LoadTextures();
	CreateSamplers();
	CreateVertexBuffers();
	CreatePingPongBuffers();
	UpdateKernelTexture();
}

void PingPongBuffers::ResizeNotify()
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

void PingPongBuffers::Update(double elapsed)
{
	if (!enableUpdate)
		Initialize();
	enableUpdate = true;
}

void PingPongBuffers::BlurPass(ComPtr<ID3D11ShaderResourceView> input)
{
	ComPtr<ID3D11Resource> pResource;
	input->GetResource(&*pResource);
	ComPtr<ID3D11Texture2D> srvTexture = pResource.QueryInterface<ID3D11Texture2D>();
	if (!srvTexture)
		return;
	
	D3D11_VIEWPORT viewport = ViewportFromTexture(srvTexture);

	pDeviceContext->RSSetViewports(1, &viewport);
	pDeviceContext->OMSetRenderTargets(1, &*pingPongBuffers[0].rtv, nullptr);

	pDeviceContext->PSSetShaderResources(0, 1, &*input);

}

void PingPongBuffers::Render()
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
	pDeviceContext->PSSetShaderResources(0, 1, &*inputSRV);
	pDeviceContext->PSSetSamplers(0, 1, &*linearSampler);

	pDeviceContext->Draw(4, 0);

	ID3D11ShaderResourceView* textures[] = { *(selectedTexture->second), *kernelSrv };
	pDeviceContext->IASetVertexBuffers(0, 1, &*outputVertBuffer, strides, offsets);

	pDeviceContext->PSSetShader(*kernelShader, nullptr, 0);
	pDeviceContext->PSSetShaderResources(1, 1, &*kernelSrv);
	pDeviceContext->PSSetConstantBuffers(0, 1, &*kernelShaderCBuffer);

	pDeviceContext->Draw(4, 0);

	// Debug draw
	pDebugDraw->Render();

	pSwapChain->Present(1, 0);
}
