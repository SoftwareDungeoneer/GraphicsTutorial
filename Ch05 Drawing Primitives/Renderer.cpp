#include "Renderer.h"

#include <vector>

#include "util.h"

Renderer::Renderer(HWND _hWnd):hWnd(_hWnd)
{
	CreateD3DDevice();
	pDebugDraw = std::make_shared<DebugDraw>(pDevice, pDeviceContext, pSwapChain);
}

Renderer::~Renderer()
{
	pDeviceContext->ClearState();
	pDeviceContext->Flush();
}

void Renderer::Resize(unsigned cx, unsigned cy)
{
	windowWidth = cx;
	windowHeight = cy;

	if (pSwapChain)
		pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

	ResizeNotify();
}

D3D11_VIEWPORT Renderer::ViewportFromTexture(ComPtr<ID3D11Texture2D>& tex)
{
	D3D11_TEXTURE2D_DESC desc;
	tex->GetDesc(&desc);
	D3D11_VIEWPORT viewport{
		0, 0,
		desc.Width * 1.f,
		desc.Height * 1.f,
		0, 1
	};
	return viewport;
}

void Renderer::CreateD3DDevice()
{
	std::vector<D3D_FEATURE_LEVEL> featureLevels{
		D3D_FEATURE_LEVEL_11_1,
		// D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL actualFeatureLevel;

	D3D_DRIVER_TYPE driverType{ D3D_DRIVER_TYPE_HARDWARE };
	auto deviceFlags{ D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG /* | D3D11_CREATE_DEVICE_DEBUGGABLE */ };

	HRESULT hr = D3D11CreateDevice(
		NULL,
		driverType,
		NULL,
		deviceFlags,
		featureLevels.data(),
		static_cast<UINT>(featureLevels.size()),
		D3D11_SDK_VERSION,
		&*pDevice,
		&actualFeatureLevel,
		&*pDeviceContext
	);

	ComPtr<IDXGIDevice1> DXGIDevice;
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory1> factory;

	pDevice->QueryInterface(&*DXGIDevice);
	DXGIDevice->GetParent(__uuidof(IDXGIAdapter1), (void**)&*adapter);
	if (adapter)
	{
		adapter->GetParent(__uuidof(IDXGIFactory1), (void**)&*factory);

		if (factory)
		{
			DXGI_SWAP_CHAIN_DESC swap_chain_desc;
			ZeroInitialize(swap_chain_desc);
			swap_chain_desc.BufferDesc.RefreshRate = DXGI_RATIONAL{ 60, 1 };
			swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swap_chain_desc.SampleDesc.Count = 1;
			swap_chain_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swap_chain_desc.BufferCount = 2;
			swap_chain_desc.OutputWindow = hWnd;
			swap_chain_desc.Windowed = TRUE;
			swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			hr = factory->CreateSwapChain(*pDevice, &swap_chain_desc, &*pSwapChain);
		}
	}
}
